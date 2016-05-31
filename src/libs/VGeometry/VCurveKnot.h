// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef VGEOMETRY_VCURVEKNOT_H
#define VGEOMETRY_VCURVEKNOT_H

#include <glm/vec2.hpp>

namespace VGeometry
{

/// \class VCurveKnot
/// \brief A struct that stores the data of one VCurve knot.
///
struct VCurveKnot
{
    /// Position of the centerline of the curve at this sample.
    ///
    glm::dvec2 position;

    /// Width (or thickness) of the curve of at this sample.
    ///
    double width;

    /// Whether this knot is a corner knot or a smooth knot
    ///
    bool isCorner;

    /// Supplementary angle between this knot and neighboring knots. Value is
    /// in [0,pi]. It is equal to zero if it is an end knot, or if three
    /// consecutive knots are aligned.
    ///
    double angle;
};

} // end namespace VGeometry

#endif // VGEOMETRY_VCURVEKNOT_H
