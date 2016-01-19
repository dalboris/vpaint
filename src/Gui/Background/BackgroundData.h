// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef BACKGROUND_DATA_H
#define BACKGROUND_DATA_H

#include "Color.h"

#include <QString>
#include <Eigen/Core>
#include <QString>
#include <QImage>

struct BackgroundData
{
    enum class SizeType {
        Cover = 0,
        Manual = 1
    };

    enum class RepeatType {
        NoRepeat = 0,
        RepeatX = 1,
        RepeatY = 2,
        Repeat = 3
    };

    Color color;
    QString imageUrl;
    Eigen::Vector2d position;
    SizeType sizeType;
    Eigen::Vector2d size;
    RepeatType repeatType;
    double opacity;
    bool hold;

    // Default background data values
    BackgroundData();

    // Are two background data equal?
    bool operator==(const BackgroundData & other) const;
    bool operator!=(const BackgroundData & other) const;
};

#endif // BACKGROUND_DATA_H
