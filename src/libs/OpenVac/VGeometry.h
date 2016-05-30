// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_VGEOMETRY_H
#define OPENVAC_VGEOMETRY_H

#include "DGeometry/Frame.h"
#include "DGeometry/KeyVertexGeometry.h"
#include "VGeometry/KeyEdgeGeometry.h"
#include "DGeometry/GeometryManager.h"

namespace OpenVac
{

namespace Geometry
{

using Frame             = DGeometry::Frame;
using KeyVertexGeometry = DGeometry::KeyVertexGeometry;
using KeyEdgeGeometry   = VGeometry::KeyEdgeGeometry;
using GeometryManager   = DGeometry::GeometryManager;

} // end namespace Geometry

} // end namespace OpenVac

#endif // OPENVAC_VGEOMETRY_H
