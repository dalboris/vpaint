// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef VIEW2DCAMERADATA_H
#define VIEW2DCAMERADATA_H

#include <QPointF>

/// \class View2DCamera
/// \brief A struct to store 2D camera data.
///
struct View2DCameraData
{
    /// Position attribute of the camera. This is equal to the 2D scene
    /// coordinate at the View's centerpoint.
    ///
    QPointF position = QPointF(0.0, 0.0);

    /// Rotation attribute of the camera. If rotation == pi/4, then the canvas
    /// appears rotated 45 degrees anti-clockwise on screen.
    ///
    double rotation = 0.0;

    /// Scale attribute of the camera. If scale == 2, then unit in scene
    /// coordiates appears as 2 pixels on screen.
    ///
    double scale = 1.0;
};

#endif // VIEW2DCAMERADATA_H
