// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef CAMERA2DDATA_H
#define CAMERA2DDATA_H

#include <QPointF>
#include <QMatrix4x4>

/// \class Camera2DData
/// \brief A struct to store 2D camera data.
///
struct Camera2DData
{
    /// Position attribute of the camera. This is equal to the position in
    /// view coordinates of the scene origin point (0.0, 0.0).
    ///
    QPointF position = QPointF(0.0, 0.0);

    /// Rotation attribute of the camera. If rotation == pi/4, then the canvas
    /// appears rotated 45 degrees anti-clockwise on screen.
    ///
    double rotation = 0.0;

    /// Scale attribute of the camera. If scale == 2, then one unit in scene
    /// coordinates appears as 2 pixels on screen.
    ///
    double scale = 1.0;

    /// Converts 2D camera data to 4x4 matrix
    ///
    QMatrix4x4 toMatrix() const;

    /// Modifies the position attributes such that the given \p scenePos
    /// appears at the given \p viewPos. After this operation, we have:
    ///
    ///     viewPos == toMatrix() * scenePos
    ///
    void translateScenePosToViewPos(const QPointF & scenePos,
                                    const QPointF & viewPos);
};

#endif // CAMERA2DDATA_H
