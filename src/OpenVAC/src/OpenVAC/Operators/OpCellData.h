// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_OPCELLDATA_H
#define OPENVAC_OPCELLDATA_H

#include <OpenVAC/Core/Memory.h>
#include <OpenVAC/Topology/CellId.h>
#include <OpenVAC/Topology/TCellData/DefineCellData.h>

namespace OpenVAC
{

class OpCellDataTrait
{
public:
    typedef CellId CellRef;
    typedef CellId KeyCellRef;
    typedef CellId InbetweenCellRef;
    typedef CellId VertexCellRef;
    typedef CellId EdgeCellRef;
    typedef CellId FaceCellRef;
    typedef CellId KeyVertexRef;
    typedef CellId KeyEdgeRef;
    typedef CellId KeyFaceRef;
    typedef CellId InbetweenVertexRef;
    typedef CellId InbetweenEdgeRef;
    typedef CellId InbetweenFaceRef;
};

OPENVAC_DEFINE_CELL_DATA(Op, OpCellDataTrait)

OPENVAC_DECLARE_SHARED_PTR(OpCellData)
OPENVAC_DECLARE_SHARED_PTR(OpKeyVertexData)
OPENVAC_DECLARE_SHARED_PTR(OpKeyEdgeData)
/* XXX TODO
OPENVAC_DECLARE_SHARED_PTR(OpKeyFaceData)
OPENVAC_DECLARE_SHARED_PTR(OpInbetweenVertexData)
OPENVAC_DECLARE_SHARED_PTR(OpInbetweenEdgeData)
OPENVAC_DECLARE_SHARED_PTR(OpInbetweenFaceData)
*/

OPENVAC_DECLARE_PTR(OpCellData)
OPENVAC_DECLARE_PTR(OpKeyVertexData)
OPENVAC_DECLARE_PTR(OpKeyEdgeData)
/* XXX TODO
OPENVAC_DECLARE_PTR(OpKeyFaceData)
OPENVAC_DECLARE_PTR(OpInbetweenVertexData)
OPENVAC_DECLARE_PTR(OpInbetweenEdgeData)
OPENVAC_DECLARE_PTR(OpInbetweenFaceData)
*/

}

#endif
