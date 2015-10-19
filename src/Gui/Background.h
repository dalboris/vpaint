// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "Color.h"
#include <QString>
#include <QImage>
#include <QList>
#include <Eigen/Core>

class Background
{
public:
    Background();

    // Getters
    bool isBackgroundColorEnabled() const;
    bool isBackgroundColorExported() const;
    Color backgroundColor() const;

    bool areBackgroundImagesEnabled() const;
    bool areBackgroundImagesExported() const;
    QStringList backgroundImagesPaths() const;
    QList<QImage> backgroundImages() const;
    int numBackgroundImages() const;
    QImage backgroundImage(int i) const;
    int numFramesPerBackgroundImage() const;
    int startFrameOfBackgroundImages() const;
    bool areBackgroundImagesHold() const;

    Eigen::Vector2d backgroundImagesPosition() const;
    Eigen::Vector2d backgroundImagesSize() const;
    double backgroundImagesOpacity() const;

    // Setters
    void setBackgroundColorEnabled(bool b);
    void setBackgroundColorExported(bool b);
    void setBackgroundColor(const Color & color);

    void setBackgroundImagesEnabled(bool b);
    void setBackgroundImagesExported(bool b);
    void setBackgroundImagesPaths(const QStringList & paths);
    void setNumFramesPerBackgroundImage(int numFrames);
    void setStartFrameOfBackgroundImages(int frame);
    void setBackgroundImagesHold(bool b);

    void setBackgroundImagesPosition(const Eigen::Vector2d & pos);
    void setBackgroundImagesSize(const Eigen::Vector2d & size);
    void setBackgroundImagesOpacity(double opacity);

    // Others
    void updateImagesFromPaths();

private:
    QImage makeImageFromPath(const QString & path);

    bool isBackgroundColorEnabled_;
    bool isBackgroundColorExported_;
    Color backgroundColor_;

    bool areBackgroundImagesEnabled_;
    bool areBackgroundImagesExported_;
    QStringList backgroundImagesPaths_;
    QList<QImage> backgroundImages_;
    int numFramesPerBackgroundImage_;
    int startFrameOfBackgroundImages_;
    bool areBackgroundImagesHold_;

    Eigen::Vector2d backgroundImagesPosition_;
    Eigen::Vector2d backgroundImagesSize_;
    double backgroundImagesOpacity_;
};

#endif // BACKGROUND_H
