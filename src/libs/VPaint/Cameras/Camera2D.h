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
};

#endif // CAMERA2D_H
