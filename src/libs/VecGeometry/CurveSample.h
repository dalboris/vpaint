// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef CURVESAMPLE_H
#define CURVESAMPLE_H

#include <glm/vec2.hpp>

/// \class CurveSample
/// \brief A struct that stores the data of one curve sample.
///
struct CurveSample
{
    /// Position of the centerline of the curve at this sample.
    ///
    glm::vec2 position;

    /// Width (or thickness) of the curve of at this sample.
    ///
    glm::vec2 width;

    /// Tangent of the curve at this sample.
    ///
    glm::vec2 tangent;

    /// Normal of the curve at this sample. The normal is pointing towards the
    /// "right side", when walking along the curve (i.e., increasing
    /// arclengths), in a 2D coordinate system where the X-axis is pointing
    /// to the right and the Y-axis is pointing down.
    ///
    /// Note that tangent and normal are redundant but provided for speed
    /// and convenience:
    ///
    ///     normal.x = - tangent.y
    ///     normal.y = + tangent.x
    ///
    glm::vec2 normal;

    /// Length between the start of the curve and this sample. More
    /// specifically, this is the sum of the length of the linear segments up
    /// to this sample.
    ///
    float arclength;

    /// Angle formed by the two segments of the curve incident to this sample.
    /// While not linearly related to curvature, this provides info which can
    /// be used in a similar way that an approximation of the curvature could.
    /// If angle == 0, then curvature us null at this sample (the three
    /// consecutive samples around this sample are aligned).
    ///
    float angle;
};

#endif // CURVESAMPLE_H
