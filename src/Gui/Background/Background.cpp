// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2016 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "Background.h"

#include "XmlStreamReader.h"
#include "XmlStreamWriter.h"
#include "CssColor.h"

#include <QDir>
#include <QFileInfo>
#include <QVector>

// Constructor
Background::Background(QObject * parent) :
    QObject(parent),
    data_(),
    cached_(false)
{
}

// Copy constructor
Background::Background(const Background & other, QObject * parent) :
    QObject(parent),
    data_(other.data_),
    cached_(false)
{
}

// Assignment operator
Background &  Background::operator=(const Background & other)
{
    setData(other.data());
    return *this;
}

// Data
const Background::Data & Background::data() const
{
    return data_;
}

void Background::setData(const Data & newData)
{
    if (data_ != newData)
    {
        data_ = newData;
        clearCache();
        emit changed();
    }
}

void Background::setData(const Background * other)
{
    if (other)
        setData(other->data());
    else
        resetData();
}

void Background::resetData()
{
    setData(Data());
}

// Color
Color Background::color() const
{
    return data_.color;
}

void Background::setColor(const Color & newColor)
{
    if (data_.color != newColor)
    {
        data_.color = newColor;
        emit colorChanged(data_.color);
        emit changed();
    }
}

// Image(s)
QString Background::imageUrl() const
{
    return data_.imageUrl;
}

void Background::setImageUrl(const QString & newUrl)
{
    if (data_.imageUrl != newUrl)
    {
        data_.imageUrl = newUrl;
        clearCache();
        emit imageUrlChanged(data_.imageUrl);
        emit changed();
    }
}

// Compute images
void Background::clearCache()
{
    filePathsPrefix_.clear();
    filePathsSuffix_.clear();
    filePathsWildcards_.clear();
    referenceFrames_.clear();
    cached_ = false;
    emit cacheCleared();
}

void Background::updateCache_() const
{
    if (!cached_)
    {
        computeCache_();
        cached_ = true;
    }
}

void Background::computeCache_() const
{
    // Default values, such that image(f) returns "" for all frames
    filePathsWildcards_.clear();
    referenceFrames_.clear();
    filePathsPrefix_.clear();
    filePathsSuffix_.clear();

    // Check that there is at most one "*" character, and
    // that it is not followed by any "/" character (todo)
    int wildcardIndex = -1;
    for (int i=0; i<data_.imageUrl.length(); ++i)
    {
        QChar c = data_.imageUrl[i];
        if (c == '*')
        {
            if (wildcardIndex == -1)
            {
                wildcardIndex = i;
            }
            else
            {
                // XXX use a QMessageBox? a QLineEdit validation?
                //     should the check be done and corrected even before
                //     this function may be reached in this invalid state?
                qDebug("more than one wildcard");
            }
        }
    }

    // Get url relative to working dir instead of document dir
    QDir dir = QDir::home();
    QString url = dir.filePath(data_.imageUrl);

    // Case without wildcard
    if (wildcardIndex == -1)
    {
        filePathsPrefix_ = url;
    }

    // Case with wildcard
    else
    {
        // Get matching files
        QDir::Filters filters = QDir::Files | QDir::Readable;
        QStringList nameFilters;
        nameFilters << data_.imageUrl;
        QFileInfoList files = dir.entryInfoList(nameFilters, filters);

        // Offset wildcardIndex to make it relative to working dir instead of
        // document dir. Here are what the values look like to help understand:
        //
        //    dir.path() = "my/dir"
        //
        //    imageUrl_ = "my/background*.png"
        //    url       = "my/dir/my/background*.png"
        //
        //    files[i].filePath() = "my/dir/my/background015.png"
        //
        //    wildcardIndex (before) = 13
        //    wildcardIndex (after)  = 20 = 13 + 6 + 1
        //
        wildcardIndex += dir.path().length() + 1;

        // Get prefix and suffix
        filePathsPrefix_ = url.left(wildcardIndex);
        filePathsSuffix_ = url.right(url.length() - wildcardIndex - 1);

        // Get wildcard values as string and int. Example:
        //   files[i].filePath() = "my/dir/my/background015.png"
        //   wildcardValue[i]    = "015"
        QStringList stringWildcards;
        QList<int> intWildcards;
        foreach(QFileInfo i, files)
        {
            // Get wildcard as string
            QString stringWildcard = i.filePath();
            stringWildcard.remove(0, filePathsPrefix_.length());
            stringWildcard.chop(filePathsSuffix_.length());

            // Try to convert to an int
            bool ok;
            int intWildcard = stringWildcard.toInt(&ok);

            // Append to lists, but only if conversion succeeded
            if (ok)
            {
                stringWildcards << stringWildcard;
                intWildcards << intWildcard;
            }
        }
        int nWildcards = stringWildcards.size(); // == intWildcards.size();

        // If there is zero match, then it's trivial
        if (nWildcards > 0) // Nothing to do if there's zero match:
                            // image(f) will return prefix + suffix for all frames
        {
            // Find minimum, maximum, and their indices
            // Note: it's possible that min = max (e.g., only one entry)
            //       it's also possible that min or max are not unique (e.g., "background01.png" and "background1.png")
            minFrame_ = intWildcards[0];
            int maxFrame = intWildcards[0];
            for (int i=1; i<nWildcards; ++i)
            {
                if (intWildcards[i] > maxFrame) {
                    maxFrame = intWildcards[i];
                }
                if (intWildcards[i] < minFrame_) {
                    minFrame_ = intWildcards[i];
                }
            }

            // The remaining step is to create an array 'filePathsWildcards_' s.t.
            // for each f in [min, max], the value filePathsWildcards_[f-min]
            // represents the stringWildcard to use for frame f.
            //
            // example:
            //    stringWildcards = [ "03", "8", "005" ]
            //    intWildcards =    [ 3, 8, 5 ]
            //    min = 3 ; indexMin = 0
            //    max = 8 ; indexMax = 1
            //
            //    If hold == true:
            //    frameToString = [ "03", "03", "005", "005", "005", "8" ]
            //                f =    3     4       5     6      7     8
            //
            //    If hold == false:
            //    frameToString = [ "03", "",   "005",   "",    "",   "8" ]
            //                f =    3     4       5     6      7     8
            //

            // Initialization (non-empty vector of empty strings)
            filePathsWildcards_ = QVector<QString>(maxFrame-minFrame_+1);
            referenceFrames_ = QVector<int>(maxFrame-minFrame_+1, 0);

            // Fill with values of existing files
            for (int i=0; i<nWildcards; ++i) {
                filePathsWildcards_[intWildcards[i] - minFrame_] = stringWildcards[i];
                referenceFrames_[intWildcards[i] - minFrame_] = intWildcards[i];
            }

            // Fill the blanks between existing files if hold == true
            if (hold())
            {
                QString lastValidWildcard = filePathsWildcards_.first();
                int lastValidIntWildcard = referenceFrames_.first();
                for (int i=1; i<filePathsWildcards_.size()-1; ++i)
                {
                    if (filePathsWildcards_[i].isEmpty()) {
                        filePathsWildcards_[i] = lastValidWildcard;
                        referenceFrames_[i] = lastValidIntWildcard;
                    }
                    else {
                        lastValidWildcard = filePathsWildcards_[i];
                        lastValidIntWildcard = referenceFrames_[i];
                    }
                }
            }
            // Fill the blanks between existing files if hold == false
            // Nothing to do for filePathsWildcards_. The blank are
            // already an empty string, which is the correct result.
            // However, for now the blanks in referenceFrames_ are 0,
            // which is not good, 0 might be a provided frame. We want
            // to make sure each blank point to an unprovided frame, so
            // we choose minFrame - 1.
            else
            {
                for (int i=1; i<filePathsWildcards_.size()-1; ++i)
                {
                    if (filePathsWildcards_[i].isEmpty()) {
                        referenceFrames_[i] = minFrame_ - 1;
                    }
                }
            }
        }
    }
}

int Background::referenceFrame(int frame) const
{
    updateCache_();

    if (referenceFrames_.isEmpty())
    {
        // All frames share the same background image
        return 0;
    }
    else
    {
        if (frame < minFrame_)
        {
            if (hold())
                return referenceFrames_.first();
            else
                return minFrame_ - 1;
        }
        else if (frame < minFrame_ + filePathsWildcards_.size())
        {
            return referenceFrames_[frame - minFrame_];
        }
        else
        {
            if (hold())
                return referenceFrames_.last();
            else
                return minFrame_ - 1;
        }
    }
}

QImage Background::image(int frame) const
{
    updateCache_();

    // Prefix
    QString filePath(filePathsPrefix_);

    // Wildcard
    if (!filePathsWildcards_.isEmpty())
    {
        if (frame < minFrame_)
        {
            if (hold())
                filePath += filePathsWildcards_.first();
        }
        else if (frame < minFrame_ + filePathsWildcards_.size())
        {
            filePath += filePathsWildcards_[frame - minFrame_];
        }
        else
        {
            if (hold())
                filePath += filePathsWildcards_.last();
        }
    }

    // Suffix
    filePath += filePathsSuffix_;

    // Read and return image
    QFileInfo fileInfo(filePath);
    if (fileInfo.exists() && fileInfo.isFile())
    {
        return QImage(filePath);
    }
    else
    {
        return QImage();
    }
}

// Position
Eigen::Vector2d Background::position() const
{
    return data_.position;
}

void Background::setPosition(const Eigen::Vector2d & newPosition)
{
    if (data_.position != newPosition)
    {
        data_.position = newPosition;
        if (data_.position[0] < -2.0) {
            data_.position[0] = -2.0;
        }
        emit positionChanged(data_.position);
        emit changed();
    }
}

// Size
Background::SizeType Background::sizeType() const
{
    return data_.sizeType;
}

Eigen::Vector2d Background::size() const
{
    return data_.size;
}

Eigen::Vector2d Background::computedSize(const Eigen::Vector2d & canvasSize) const
{
    switch (sizeType())
    {
    case SizeType::Cover:
        return canvasSize;
    case SizeType::Manual:
        return size();
    default:
        return size();
    }
}

void Background::setSizeType(SizeType newSizeType)
{
    if(data_.sizeType != newSizeType)
    {
        data_.sizeType = newSizeType;
        emit sizeTypeChanged(data_.sizeType);
        emit changed();
    }
}

void Background::setSize(const Eigen::Vector2d & newSize)
{
    if (data_.size != newSize)
    {
        data_.size = newSize;
        emit sizeChanged(data_.size);
        emit changed();
    }
}

// Repeat
Background::RepeatType Background::repeatType() const
{
    return data_.repeatType;
}

void Background::setRepeatType(RepeatType newRepeatType)
{
    if (data_.repeatType != newRepeatType)
    {
        data_.repeatType = newRepeatType;
        emit repeatTypeChanged(data_.repeatType);
        emit changed();
    }
}

bool Background::repeatX() const
{
    switch (repeatType())
    {
    case RepeatType::RepeatX:
    case RepeatType::Repeat:
        return true;
    default:
        return false;
    }
}

bool Background::repeatY() const
{
    switch (repeatType())
    {
    case RepeatType::RepeatY:
    case RepeatType::Repeat:
        return true;
    default:
        return false;
    }
}

// Opacity
double Background::opacity() const
{
    return data_.opacity;
}

void Background::setOpacity(double newOpacity)
{
    if (data_.opacity != newOpacity)
    {
        data_.opacity = newOpacity;
        emit opacityChanged(data_.opacity);
        emit changed();
    }
}

// Hold
bool Background::hold() const
{
    return data_.hold;
}

void Background::setHold(bool newHold)
{
    if (data_.hold != newHold)
    {
        data_.hold = newHold;
        clearCache();
        emit holdChanged(data_.hold);
        emit changed();
    }
}

namespace
{
QString toString(double x)
{
    return QString().setNum(x);
}
}

void Background::write(XmlStreamWriter & xml)
{
    // Color
    xml.writeAttribute("color", "rgba(" + toString(color().red()) + "," +
                                          toString(color().green()) + "," +
                                          toString(color().blue()) + "," +
                                          toString(color().alphaF()) + ")");

    // Image
    xml.writeAttribute("image", imageUrl());

    // Position
    xml.writeAttribute("position", toString(position()[0]) + " " +
                                   toString(position()[1]));

    // Size
    switch (sizeType())
    {
    case SizeType::Cover:
        xml.writeAttribute("size", "cover");
        break;
    case SizeType::Manual:
        xml.writeAttribute("size", toString(size()[0]) + " " +
                                   toString(size()[1]));
        break;
    }

    // Repeat
    switch (repeatType())
    {
    case RepeatType::NoRepeat:
        xml.writeAttribute("repeat", "norepeat");
        break;
    case RepeatType::RepeatX:
        xml.writeAttribute("repeat", "repeatx");
        break;
    case RepeatType::RepeatY:
        xml.writeAttribute("repeat", "repeaty");
        break;
    case RepeatType::Repeat:
        xml.writeAttribute("repeat", "repeat");
        break;
    }

    // Opacity
    xml.writeAttribute("opacity", toString(opacity()));

    // Hold
    xml.writeAttribute("hold", hold() ? "yes" : "no");
}

void Background::read(XmlStreamReader & xml)
{
    // Default data values
    Data data;

    // Color
    if(xml.attributes().hasAttribute("color"))
    {
        CssColor c(xml.attributes().value("color").toString());
        data.color = c.toColor();
    }

    // Image
    if(xml.attributes().hasAttribute("image"))
    {
        data.imageUrl = xml.attributes().value("image").toString();
    }

    // Position
    if(xml.attributes().hasAttribute("position"))
    {
        QStringList sl =  xml.attributes().value("position").toString().split(' ');
        data.position = Eigen::Vector2d(sl[0].toDouble(), sl[1].toDouble());
    }

    // Size
    if(xml.attributes().hasAttribute("size"))
    {
        QString sizeString =  xml.attributes().value("size").toString();
        if (sizeString == "cover")
        {
            data.sizeType = SizeType::Cover;
        }
        else
        {
            QStringList sl =  sizeString.split(' ');
            data.sizeType = SizeType::Manual;
            data.size = Eigen::Vector2d(sl[0].toDouble(), sl[1].toDouble());
        }
    }

    // Repeat
    if(xml.attributes().hasAttribute("repeat"))
    {
        QString repeatString =  xml.attributes().value("repeat").toString();
        if (repeatString == "norepeat")
        {
            data.repeatType = RepeatType::NoRepeat;
        }
        else if (repeatString == "repeatx")
        {
            data.repeatType = RepeatType::RepeatX;
        }
        else if (repeatString == "repeaty")
        {
            data.repeatType = RepeatType::RepeatY;
        }
        else if (repeatString == "repeat")
        {
            data.repeatType = RepeatType::Repeat;
        }
    }

    // Opacity
    if(xml.attributes().hasAttribute("opacity"))
    {
        data.opacity = xml.attributes().value("opacity").toDouble();
    }

    // Hold
    if(xml.attributes().hasAttribute("hold"))
    {
        QString holdString =  xml.attributes().value("hold").toString();
        if (holdString == "yes")
        {
            data.hold = true;
        }
        else if (holdString == "no")
        {
            data.hold = false;
        }
    }

    // Unknown
    xml.skipCurrentElement();

    // Set Data
    setData(data);
}
