// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "Camera2DData.h"

#include <cmath>

QMatrix4x4 Camera2DData::toMatrix() const
{
    QMatrix4x4 res;
    res.translate(position.x(), position.y());
    res.rotate(rotation * 180.0 / M_PI, 0.0, 0.0, 1.0);
    res.scale(scale);
    return res;
}

void Camera2DData::translateScenePosToViewPos(
        const QPointF & scenePos,
        const QPointF & viewPos)
{
    const QPointF currentViewPos = toMatrix() * scenePos;
    position += viewPos - currentViewPos;
}
