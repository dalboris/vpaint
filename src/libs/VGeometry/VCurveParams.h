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
    /// Max angle between consecutive samples
    ///
    double maxSampleAngle = 0.05; // approx. PI/64
};

} // end namespace VGeometry

#endif // VGEOMETRY_VCURVEPARAMS_H
