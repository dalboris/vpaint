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
/// \brief A class to store 2D camera data.
///
struct View2DCameraData
{
    QPointF position; ///< 2D scene coordinate at the View's centerpoint
    double rotation;  ///< (rotation == pi/4) => the canvas appears rotated 45 degrees anti-clockwise
    double scale;     ///< (scale == 2)       => each vector unit appears as 2 pixels on screen
};

#endif // VIEW2DCAMERADATA_H
