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

#ifndef BACKGROUND_DATA_H
#define BACKGROUND_DATA_H

#include "../Color.h"

#include <QString>
#include <Eigen/Core>
#include <QString>
#include <QImage>
#include "VAC/vpaint_global.h"

struct Q_VPAINT_EXPORT BackgroundData
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

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
