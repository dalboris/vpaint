// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "Background.h"

#include <QFileInfo>

Background::Background() :
    color_(Qt::white),
    imageUrl_("/home/boris/test.png"), // XXX change back to ""
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

QImage Background::image(int /* frame */) const
{
    // for now, ignore frame, and don't cache image
    QFileInfo checkFile(imageUrl());
    if (checkFile.exists() && checkFile.isFile())
    {
        return QImage(imageUrl());
    }
    else
    {
        return QImage();
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

/*
void Background::updateImagesFromPaths()
{
    backgroundImages_.clear();
    foreach(QString path, backgroundImagesPaths_)
    {
        backgroundImages_ << QImage(path);
    }
}
*/
