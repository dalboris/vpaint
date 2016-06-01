// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef VGEOMETRY_VCURVEPARAMS_H
#define VGEOMETRY_VCURVEPARAMS_H

namespace VGeometry
{

struct VCurveParams
{
    /// Max angle between consecutive samples. This means that subdivision
    /// stops only when all angles between samples are < maxSampleAngle;
    ///
    double maxSampleAngle = 0.05; // approx. PI / 64

    /// Max number of sample subdivisions between two knots. This means that we
    /// will never do more subdivisions than this number, even if there are
    /// still samples whose angle is > maxSampleAngle.
    ///
    double maxNumSubdivision = 6; // i.e., max 2^6 = 64 samples between two knots

    /// Variable to specify whether or not knots should be automatically
    /// added or removed.
    ///
    bool autoKnot = true;

    /// If auto-knot is true, then this variable controls the minimum
    /// angle between two knots. If the angle is bigger than that, then
    /// the knot is automatically removed.
    ///
    double minKnotAngle = 0.2; // approx. PI / 16

    /// If auto-knot is true, then this variable controls the maximum
    /// angle at a smooth knots. If the angle is bigger than that, either
    /// the more smooth knots are added, or the knot is converted
    /// into a corner knot, depending on the situation at hand.
    ///
    double maxSmoothKnotAngle = 1.57; // approx. PI / 2
};

} // end namespace VGeometry

#endif // VGEOMETRY_VCURVEPARAMS_H
