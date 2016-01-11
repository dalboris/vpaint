// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2015 Boris Dalstein <dalboris@gmail.com>
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

Background::Background() :
    color_(Qt::white),
    imageUrl_(""),
    position_(0.0, 0.0),
    sizeType_(Cover),
    size_(1280.0, 720.0),
    repeatType_(NoRepeat),
    opacity_(1.0),
    hold_(true)
{
}

// Assignement operator. Re-implemented to emit changed().
void Background::operator=(const Background & other)
{
    color_ = other.color_;
    imageUrl_ = other.imageUrl_;
    position_ = other.position_;
    sizeType_ = other.sizeType_;
    size_ = other.size_;
    repeatType_ = other.repeatType_;
    opacity_ = other.opacity_;
    hold_ = other.hold_;

    emit changed();
}

// Color
Color Background::color() const
{
    return color_;
}

void Background::setColor(const Color & newColor)
{
    color_ = newColor;

    emit colorChanged(color_);
    emit changed();
}

// Image(s)
QString Background::imageUrl() const
{
    return imageUrl_;
}

void Background::setImageUrl(const QString & newUrl)
{
    imageUrl_ = newUrl;

    emit imageUrlChanged(imageUrl_);
    emit changed();
}

QImage Background::image(int frame) const
{
    // Check that there is at most one "*" character, and
    // that it is not followed by any "/" character (todo)
    int wildcardIndex = -1;
    QString url = imageUrl();
    for (int i=0; i<url.length(); ++i)
    {
        QChar c = url[i];
        if (c == '*')
        {
            if (wildcardIndex == -1)
            {
                wildcardIndex = i;
            }
            else
            {
                // XXX use a QMessageBox? a QLineEdit validation?
                qDebug("more than one wildcard");
                return QImage();
            }
        }
    }

    // Set current directory
    QDir dir = QDir::home();

    // Case without wildcard
    if (wildcardIndex == -1)
    {
        QFileInfo fileInfo(dir.filePath(url));
        if (fileInfo.exists() && fileInfo.isFile())
        {
            return QImage(fileInfo.filePath());
        }
        else
        {
            return QImage();
        }
    }

    // Case with wildcard
    else
    {
        // Get matching files
        QDir::Filters filters = QDir::Files | QDir::Readable;
        QStringList nameFilters;
        nameFilters << imageUrl();
        QFileInfoList files = dir.entryInfoList(nameFilters, filters);

        // Offset wildcardIndex and extend url. Why? Because:
        //  BEFORE
        //    url = my/background*.png
        //    wildcardIndex = 13
        //    dir.path() = "my/dir"
        //    files[i].filePath() = "my/dir/my/background015.png"
        //
        //  AFTER
        //    url = my/dir/my/background*.png
        //    wildcardIndex = 20
        url = dir.filePath(url);
        wildcardIndex += dir.path().length() + 1;

        // Get prefix and suffix
        QString prefix = url.left(wildcardIndex);
        QString suffix = url.right(url.length() - wildcardIndex - 1);

        // Get wildcard values as string and int. Example:
        //   files[i].filePath() = "my/dir/my/background015.png"
        //   wildcardValue[i]    = "015"
        QStringList stringWildcards;
        QList<int> intWildcards;
        foreach(QFileInfo i, files)
        {
            // Get wildcard as string
            QString stringWildcard = i.filePath();
            stringWildcard.remove(0, prefix.length());
            stringWildcard.chop(suffix.length());

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
        if (nWildcards == 0)
        {
            // Last chance to find something: try without the wildcard
            QFileInfo fileInfo(prefix + suffix);
            if (fileInfo.exists() && fileInfo.isFile())
            {
                return QImage(fileInfo.filePath());
            }
            // Nope, there's really nothing matching
            else
            {
                return QImage();
            }
        }
        // If there is one match or more, then do the more complex stuff
        else
        {
            // Find minimum, maximum, and their indices
            // Note: it's possible that min = max (e.g., only one entry)
            //       it's also possible that min or max are not unique (e.g., "background01.png" and "background1.png")
            int min = intWildcards[0];
            int max = intWildcards[0];
            for (int i=0; i<nWildcards; ++i)
            {
                if (intWildcards[i] > max) {
                    max = intWildcards[i];
                }
                if (intWildcards[i] < min) {
                    min = intWildcards[i];
                }
            }

            // Create an array frameToString s.t. for each f in [min, max],
            // the value frameToString[f-min] represent the stringWildcard
            // to use for frame f.
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
            QVector<QString> frameToString(max-min+1); // initialize with empty strings, and is never empty
            for (int i=0; i<nWildcards; ++i) {
                frameToString[intWildcards[i] - min] = stringWildcards[i];
            }
            // Here, we know frameToString.first() and frameToString.last() are non-empty string
            if (hold())
            {
                QString s = frameToString.first();
                for (int i=1; i<frameToString.size()-1; ++i)
                {
                    if (frameToString[i].isEmpty()) {
                        frameToString[i] = s;
                    }
                    else {
                        s = frameToString[i];
                    }
                }
            }

            // Get file path for this frame
            // XXX The lines of code below should be the only one done in Background::image(int frame)
            // All the lines of code above should be done in setImageUrl() and the result cached
            QString filePath = prefix;
            if (frame < min) {
                filePath += frameToString.first();
            }
            else if (frame > max) {
                filePath += frameToString.last();
            }
            else {
                filePath += frameToString[frame - min];
            }
            filePath += suffix;

            // Read and return image
            QFileInfo fileInfo(filePath);
            if (fileInfo.exists() && fileInfo.isFile()) {
                return QImage(filePath);
            }
            else {
                return QImage();
            }
        }
    }
}

// Position
Eigen::Vector2d Background::position() const
{
    return position_;
}

void Background::setPosition(const Eigen::Vector2d & newPosition)
{
    position_ = newPosition;

    emit positionChanged(position_);
    emit changed();
}

// Size
Background::SizeType Background::sizeType() const
{
    return sizeType_;
}

Eigen::Vector2d Background::size() const
{
    return size_;
}

Eigen::Vector2d Background::computedSize(const Eigen::Vector2d & canvasSize) const
{
    switch (sizeType())
    {
    case Background::Cover:
        return canvasSize;
    case Background::Manual:
        return size();
    default:
        return size();
    }
}

void Background::setSizeType(SizeType newSizeType)
{
    sizeType_ = newSizeType;

    emit sizeTypeChanged(sizeType_);
    emit changed();
}

void Background::setSize(const Eigen::Vector2d & newSize)
{
    size_ = newSize;

    emit sizeChanged(size_);
    emit changed();
}

// Repeat
Background::RepeatType Background::repeatType() const
{
    return repeatType_;
}

void Background::setRepeatType(RepeatType newRepeatType)
{
    repeatType_ = newRepeatType;

    emit repeatTypeChanged(repeatType_);
    emit changed();
}

bool Background::repeatX() const
{
    return repeatType() & RepeatX;
}

bool Background::repeatY() const
{
    return repeatType() & RepeatY;
}

// Opacity
double Background::opacity() const
{
    return opacity_;
}

void Background::setOpacity(double newOpacity)
{
    opacity_ = newOpacity;

    emit opacityChanged(opacity_);
    emit changed();
}

// Hold
bool Background::hold() const
{
    return hold_;
}

void Background::setHold(bool newHold)
{
    hold_ = newHold;

    emit holdChanged(hold_);
    emit changed();
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
    case Cover:
        xml.writeAttribute("size", "cover");
        break;
    case Manual:
        xml.writeAttribute("size", toString(size()[0]) + " " +
                                   toString(size()[1]));
        break;
    }

    // Repeat
    switch (repeatType())
    {
    case NoRepeat:
        xml.writeAttribute("repeat", "norepeat");
        break;
    case RepeatX:
        xml.writeAttribute("repeat", "repeatx");
        break;
    case RepeatY:
        xml.writeAttribute("repeat", "repeaty");
        break;
    case Repeat:
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
    // Reset to default
    *this = Background();

    // Color
    if(xml.attributes().hasAttribute("color"))
    {
        CssColor c(xml.attributes().value("color").toString());
        setColor(c.toColor());
    }

    // Image
    if(xml.attributes().hasAttribute("image"))
    {
        setImageUrl(xml.attributes().value("image").toString());
    }

    // Position
    if(xml.attributes().hasAttribute("position"))
    {
        QStringList sl =  xml.attributes().value("position").toString().split(' ');
        setPosition(Eigen::Vector2d(sl[0].toDouble(), sl[1].toDouble()));
    }

    // Size
    if(xml.attributes().hasAttribute("size"))
    {
        QString sizeString =  xml.attributes().value("size").toString();
        if (sizeString == "cover")
        {
            setSizeType(Cover);
        }
        else
        {
            setSizeType(Manual);
            QStringList sl =  sizeString.split(' ');
            setSize(Eigen::Vector2d(sl[0].toDouble(), sl[1].toDouble()));
        }
    }

    // Repeat
    if(xml.attributes().hasAttribute("repeat"))
    {
        QString repeatString =  xml.attributes().value("repeat").toString();
        if (repeatString == "norepeat")
        {
            setRepeatType(NoRepeat);
        }
        else if (repeatString == "repeatx")
        {
            setRepeatType(RepeatX);
        }
        else if (repeatString == "repeaty")
        {
            setRepeatType(RepeatY);
        }
        else if (repeatString == "repeat")
        {
            setRepeatType(Repeat);
        }
    }

    // Opacity
    if(xml.attributes().hasAttribute("opacity"))
    {
        setOpacity(xml.attributes().value("opacity").toDouble());
    }

    // Hold
    if(xml.attributes().hasAttribute("hold"))
    {
        QString holdString =  xml.attributes().value("hold").toString();
        if (holdString == "yes")
        {
            setHold(true);
        }
        else if (holdString == "no")
        {
            setHold(false);
        }
    }

    // Unknown
    xml.skipCurrentElement();
}
