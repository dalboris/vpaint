// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

/// \file OpenVac/Geometry.h
///
/// In this file are declared all geometric types to be used by OpenVac.
/// This file is meant to be customized by users of OpenVac.

#ifndef OPENVAC_GEOMETRY_H
#define OPENVAC_GEOMETRY_H

#include "VecGeometry/EdgeGeometry.h"

#include <OpenVac/Geometry/Frame.h>
#include <vector>

namespace OpenVac
{

/// \namespace OpenVac::Geometry
/// \brief Customizable definition of geometric types.
///
/// In this namespace are declared all geometric types to be used by OpenVac. Feel
/// free to change these definitions to use your own types instead.
///
/// The following types must be declared:
///
///   - Frame
///   - KeyVertexGeometry
///   - KeyEdgeGeometry
///   - GeometryManager
///
namespace Geometry
{

/* OpenVac default geometry
 *
using Point = double[2];
using Curve = std::vector<Point>;

using Frame = OpenVac::Frame;
class KeyVertexGeometry { Point pos; };
class KeyEdgeGeometry { Curve curve; };
class GeometryManager {};
 *
 */

/* VPaint geometry */

using Frame = OpenVac::Frame;
class KeyVertexGeometry {};
using KeyEdgeGeometry = ::EdgeGeometry;
class GeometryManager {};

} // end namespace Geometry

} // end namespace OpenVac

#endif // OPENVAC_GEOMETRY_H
