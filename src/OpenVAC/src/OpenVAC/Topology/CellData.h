// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_CELLDATA_H
#define OPENVAC_CELLDATA_H

#include <OpenVAC/Topology/CellHandle.h>
#include <OpenVAC/Topology/TCellData/TCellData.h>
#include <OpenVAC/Topology/TCellData/TKeyVertexData.h>
#include <OpenVAC/Topology/TCellData/TKeyEdgeData.h>

#define OPENVAC_USING_CELLHANDLE_AS_CELLREF_(CellType) \
    typedef CellType##Handle<Geometry> CellType##Ref;

#define OPENVAC_DECLARE_CELL_DATA_(CellType) \
    template <class Geometry> \
    using CellType##Data = T##CellType##Data<HandlesAsRefs<Geometry>, Geometry>;

namespace OpenVAC
{

template <class Geometry>
class HandlesAsRefs
{
public:
    OPENVAC_FOREACH_CELL_TYPE(OPENVAC_USING_CELLHANDLE_AS_CELLREF_)
};

OPENVAC_FOREACH_CELL_DATA_TYPE(OPENVAC_DECLARE_CELL_DATA_)

}

#endif // OPENVAC_CELLDATA_H
