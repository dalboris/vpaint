// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "KeyEdgeGeometry.h"

namespace VGeometry
{

KeyEdgeGeometry::KeyEdgeGeometry()
{

}

void KeyEdgeGeometry::beginFit()
{
    curve_.beginFit();
}

void KeyEdgeGeometry::continueFit(const VCurveInputSample & inputSample)
{
    curve_.continueFit(inputSample);
}

void KeyEdgeGeometry::endFit()
{
    curve_.endFit();
}

const VCurve & KeyEdgeGeometry::curve() const
{
    return curve_;

}

} // end namespace VGeometry
