// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef VGEOMETRY_KEYEDGEGEOMETRYSAMPLE_H
#define VGEOMETRY_KEYEDGEGEOMETRYSAMPLE_H

#include <glm/vec2.hpp>

namespace VGeometry
{

/// \class KeyEdgeGeometrySample
/// \brief A class that stores the data of one KeyEdgeGeometry sample
///
/// This geometry can be seen as a QuadStrip. It is the geometry after
/// applying join/cap style.
///
struct KeyEdgeGeometrySample
{
    glm::dvec2 left;
    glm::dvec2 right;
};

} // end namespace VGeometry


#endif // VGEOMETRY_KEYEDGEGEOMETRYSAMPLE_H
