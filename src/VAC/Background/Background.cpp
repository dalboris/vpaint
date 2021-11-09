// Copyright (C) 2012-2019 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "Background.h"

#include "BackgroundUrlValidator.h"

#include "../XmlStreamReader.h"
#include "../XmlStreamWriter.h"
#include "../CssColor.h"

#include "../Global.h"

#include <QDir>
#include <QFileInfo>
#include <QVector>
#include <QTextStream>

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
        BackgroundUrlValidator::fixupUrl(data_.imageUrl);
        clearCache_();
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
        BackgroundUrlValidator::fixupUrl(data_.imageUrl);
        clearCache_();
        emit imageUrlChanged(data_.imageUrl);
        emit changed();
    }
}

// Compute images
void Background::clearCache_()
{
    filePathsPrefix_.clear();
    filePathsSuffix_.clear();
    filePathsWildcards_.clear();
    referenceFrames_.clear();
    cached_ = false;
    emit cacheCleared();
}

void Background::clearCache()
{
    clearCache_();
    emit changed();
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

    // Get url relative to working dir
    QDir dir = global()->documentDir();
    QString url = dir.filePath(data_.imageUrl);

    // Case without wildcard
    if (!url.contains('*'))
    {
        filePathsPrefix_ = url;
    }

    // Case with wildcard
    //
    // Note: At this point, we are guaranteed that there is one and
    // only one '*', and that it is not followed by '/'
    else
    {
        // Get url relative to parent dir
        QFileInfo fileInfo(url);
        QDir parentDir = fileInfo.dir();
        url = fileInfo.fileName();

        // Store path of parent dir in cached prefix
        filePathsPrefix_ = parentDir.path() + "/";

        // Get matching files
        QStringList nameFilters(url);
        QDir::Filters filters = QDir::Files | QDir::Readable;
        QFileInfoList files = parentDir.entryInfoList(nameFilters, filters);

        // Get wildcard index
        int wildcardIndex = url.indexOf('*');

        // Cache prefix and suffix
        filePathsPrefix_ += url.left(wildcardIndex);
        filePathsSuffix_ = url.right(url.length() - wildcardIndex - 1);

        // Get wildcard values as string and int.
        // Example:
        //   files[i].fileName() = "image015.png"
        //   stringWildcards[i]  = "015"
        //   intWildcards[i]     = 15
        QStringList stringWildcards;
        QList<int> intWildcards;
        for(QFileInfo i: files)
        {
            // Get wildcard as string
            QString fileName = i.fileName();
            QString stringWildcard = fileName.mid(
                        wildcardIndex, fileName.length() - url.length() + 1);

            // Append to lists, if can convert to an int
            bool ok;
            int intWildcard = stringWildcard.toInt(&ok);
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

QString Background::resolvedImageFilePath(int frame) const
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

    // Return resolved url
    return filePath;
}

QImage Background::image(int frame) const
{
    // Resolve url
    QString filePath = resolvedImageFilePath(frame);

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
        clearCache_();
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

void Background::exportSVG(int frame, QTextStream & out,
                           double canvasLeft, double canvasTop,
                           double canvasWidth, double canvasHeight)
{
    // Get linked image info
    int linkedImageWidth = 0;
    int linkedImageHeight = 0;
    QString linkedImageAbsoluteFilePath;
    QString linkedImageFilePath = resolvedImageFilePath(frame);
    QFileInfo fileInfo(linkedImageFilePath);
    if (fileInfo.exists() && fileInfo.isFile())
    {
        QImage image(linkedImageFilePath);
        linkedImageWidth = image.width();
        linkedImageHeight = image.height();
        linkedImageAbsoluteFilePath = fileInfo.absoluteFilePath();
    }

    // Get drawn image info
    Eigen::Vector2d imageComputedSize = computedSize(
                Eigen::Vector2d(canvasWidth, canvasHeight));
    double imageLeft = position()[0];
    double imageTop = position()[1];
    double imageWidth = imageComputedSize[0];
    double imageHeight = imageComputedSize[1];
    if (repeatX())
    {
        imageLeft = canvasLeft;
        imageWidth = canvasWidth;
    }
    if (repeatY())
    {
        imageTop = canvasTop;
        imageHeight = canvasHeight;
    }

    // Set SVG string to write
    QString s = QString(
                "<defs>\n"
                "  <pattern \n"
                "    id=\"backgroundpattern\"\n"
                "    width=\"%1\"\n"
                "    height=\"%2\"\n"
                "    patternUnits=\"userSpaceOnUse\"\n"
                "    patternTransform=\"translate(%3,%4) scale(%5,%6)\" >\n"
                "    <image\n"
                "         y=\"0\"\n"
                "         x=\"0\"\n"
                "         width=\"%7\"\n"
                "         height=\"%8\"\n"
                "         xlink:href=\"file://%9\" />\n"
                "  </pattern>\n"
                "</defs>\n"
                "<rect\n"
                "  id=\"backgroundcolor\"\n"
                "  x=\"%10\"\n"
                "  y=\"%11\"\n"
                "  width=\"%12\"\n"
                "  height=\"%13\"\n"
                "  style=\"fill:%14;fill-opacity:%15;stroke:none\" />\n"
                "<rect\n"
                "  id=\"backgroundimage\"\n"
                "  x=\"%16\"\n"
                "  y=\"%17\"\n"
                "  width=\"%18\"\n"
                "  height=\"%19\"\n"
                "  style=\"fill:url(#backgroundpattern);fill-opacity:%20\" />\n")

            // Pattern
            .arg(linkedImageWidth)
            .arg(linkedImageHeight)
            .arg(position()[0])
            .arg(position()[1])
            .arg(linkedImageWidth == 0 ? 0.0 : imageComputedSize[0] / linkedImageWidth)
            .arg(linkedImageHeight == 0 ? 0.0 : imageComputedSize[1] / linkedImageHeight)
            .arg(linkedImageWidth)
            .arg(linkedImageHeight)
            .arg(linkedImageAbsoluteFilePath)

            // Background color
            .arg(canvasLeft)
            .arg(canvasTop)
            .arg(canvasWidth)
            .arg(canvasHeight)
            .arg(color().name())
            .arg(color().alphaF())

            // Background image
            .arg(imageLeft)
            .arg(imageTop)
            .arg(imageWidth)
            .arg(imageHeight)
            .arg(opacity());

    // Write to file
    out << s;
}

void Background::relativeRemap(const QDir & oldDir, const QDir & newDir)
{
    QString url = imageUrl();
    if(!url.isEmpty() && oldDir.isRelativePath(url))
    {
        QString oldFilepath = oldDir.filePath(url);
        QString newFilepath = newDir.relativeFilePath(oldFilepath);
        setImageUrl(newFilepath);
    }
}

