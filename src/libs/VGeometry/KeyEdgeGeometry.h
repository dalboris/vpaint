// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef VGEOMETRY_KEYEDGEGEOMETRY_H
#define VGEOMETRY_KEYEDGEGEOMETRY_H

#include "VCurve.h"

namespace VGeometry
{

/// \class KeyEdgeGeometry
/// \brief A class that builds and stores the geometry of a key edge.
///
class KeyEdgeGeometry
{
public:
    KeyEdgeGeometry();

    void beginFit();
    void continueFit(const VCurveInputSample & inputSample);
    void endFit();

    const VCurve & curve() const;

private:
    VCurve curve_;
};

} // end namespace VGeometry

#endif // VGEOMETRY_KEYEDGEGEOMETRY_H
