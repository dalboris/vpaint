// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "Background.h"

Background::Background() :
    isBackgroundColorEnabled_(true),
    isBackgroundColorExported_(true),
    backgroundColor_(255, 255, 255),

    areBackgroundImagesEnabled_(false),
    areBackgroundImagesExported_(false),
    backgroundImagesPaths_(),
    backgroundImages_(),
    numFramesPerBackgroundImage_(1),
    startFrameOfBackgroundImages_(0),
    areBackgroundImagesHold_(),

    backgroundImagesPosition_(0,0),
    backgroundImagesSize_(640,480),
    backgroundImagesOpacity_(1.0)
{

}


// Getters
bool Background::isBackgroundColorEnabled() const
{
    return isBackgroundColorEnabled_;
}

Color Background::backgroundColor() const
{
    return backgroundColor_;
}

bool Background::areBackgroundImagesEnabled() const
{
    return areBackgroundImagesEnabled_;
}

bool Background::areBackgroundImagesExported() const
{
    return areBackgroundImagesExported_;
}

QStringList Background::backgroundImagesPaths() const
{
    return backgroundImagesPaths_;
}

QList<QImage> Background::backgroundImages() const
{
    return backgroundImages_;
}

int Background::numBackgroundImages() const
{
    return backgroundImages_.size();
}

QImage Background::backgroundImage(int i) const
{
    return backgroundImages_[i];
}

int Background::numFramesPerBackgroundImage() const
{
    return numFramesPerBackgroundImage_;
}

int Background::startFrameOfBackgroundImages() const
{
    return startFrameOfBackgroundImages_;
}

bool Background::areBackgroundImagesHold() const
{
    return areBackgroundImagesHold_;
}


Eigen::Vector2d Background::backgroundImagesPosition() const
{
    return backgroundImagesPosition_;
}

Eigen::Vector2d Background::backgroundImagesSize() const
{
    return backgroundImagesSize_;
}

double Background::backgroundImagesOpacity() const
{
    return backgroundImagesOpacity_;
}


// Setters
void Background::setBackgroundColorEnabled(bool b)
{
    isBackgroundColorEnabled_ = b;
}

void Background::setBackgroundColorExported(bool b)
{
    isBackgroundColorExported_ = b;
}

void Background::setBackgroundColor(const Color & color)
{
    backgroundColor_ = color;
}


void Background::setBackgroundImagesEnabled(bool b)
{
    areBackgroundImagesEnabled_ = b;
}

void Background::setBackgroundImagesExported(bool b)
{
    areBackgroundImagesExported_ = b;
}

void Background::setBackgroundImagesPaths(const QStringList & paths)
{
    backgroundImagesPaths_ = paths;
    updateImagesFromPaths();
}

void Background::setNumFramesPerBackgroundImage(int numFrames)
{
    numFramesPerBackgroundImage_ = numFrames;
}

void Background::setStartFrameOfBackgroundImages(int frame)
{
    startFrameOfBackgroundImages_ = frame;
}

void Background::setBackgroundImagesHold(bool b)
{
    areBackgroundImagesHold_ = b;
}


void Background::setBackgroundImagesPosition(const Eigen::Vector2d & pos)
{
    backgroundImagesPosition_ = pos;
}

void Background::setBackgroundImagesSize(const Eigen::Vector2d & size)
{
    backgroundImagesSize_ = size;
}

void Background::setBackgroundImagesOpacity(double opacity)
{
    backgroundImagesOpacity_ = opacity;
}

void Background::updateImagesFromPaths()
{
    backgroundImages_.clear();
    foreach(QString path, backgroundImagesPaths_)
    {
        backgroundImages_ << makeImageFromPath(path);
    }
}

QImage Background::makeImageFromPath(const QString & path)
{
    return QImage(path);
}
