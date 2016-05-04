// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef CAMERA2D_H
#define CAMERA2D_H

#include "Core/DataObject.h"
#include "Cameras/Camera2DData.h"

#include <QMatrix4x4>

/// \class Camera2D
/// \brief A class to represent a 2D camera object.
///
class Camera2D: public DataObject<Camera2DData>
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(Camera2D)


public:
    /// Constructs a View2DCamera.
    ///
    Camera2D();

    /// Converts 2D camera data to 4x4 matrix
    ///
    QMatrix4x4 toMatrix() const;

    /// Returns the position of the camera.
    ///
    QPointF position() const;

    /// Sets the position of the camera. Emits changed().
    ///
    void setPosition(const QPointF & p);

    /// Returns the rotation of the camera.
    ///
    double rotation() const;

    /// Sets the rotation of the camera. Emits changed().
    ///
    void setRotation(double r);

    /// Returns the scale of the camera.
    ///
    double scale() const;

    /// Sets the scale of the camera. Emits changed().
    ///
    void setScale(double s);
};

#endif // CAMERA2D_H
