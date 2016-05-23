// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "EdgeGeometry.h"


EdgeGeometry::EdgeGeometry()
{

}

void EdgeGeometry::beginFit()
{
    curve_.clear();
}

void EdgeGeometry::addFitInputSample(const VecCurveInputSample & inputSample)
{
    curve_.addSample(inputSample);
}

void EdgeGeometry::endFit()
{
}

const VecCurve & EdgeGeometry::curve() const
{
    return curve_;

}
