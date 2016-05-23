// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef VECGEOMETRY_EDGEGEOMETRY_H
#define VECGEOMETRY_EDGEGEOMETRY_H

#include "VecCurve.h"

/// \class EdgeGeometry
/// \brief A class that builds and stores the geometry of a key edge.
///
class EdgeGeometry
{
public:
    EdgeGeometry();

    void beginFit();
    void addFitInputSample(const VecCurveInputSample & inputSample);
    void endFit();

    const VecCurve & curve() const;

private:
    VecCurve curve_;
};

#endif // VECGEOMETRY_EDGEGEOMETRY_H
