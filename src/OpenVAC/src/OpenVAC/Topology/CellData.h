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
#include <OpenVAC/Topology/TCellData/DefineCellData.h>

namespace OpenVAC
{

class CellDataTrait
{
public:
    typedef CellHandle            CellRef;
    typedef KeyCellHandle         KeyCellRef;
    typedef InbetweenCellHandle   InbetweenCellRef;
    typedef VertexCellHandle      VertexCellRef;
    typedef EdgeCellHandle        EdgeCellRef;
    typedef FaceCellHandle        FaceCellRef;
    typedef KeyVertexHandle       KeyVertexRef;
    typedef KeyEdgeHandle         KeyEdgeRef;
    typedef KeyFaceHandle         KeyFaceRef;
    typedef InbetweenVertexHandle InbetweenVertexRef;
    typedef InbetweenEdgeHandle   InbetweenEdgeRef;
    typedef InbetweenFaceHandle   InbetweenFaceRef;
};

OPENVAC_DEFINE_CELL_DATA(/* No prefix */, CellDataTrait)

}

#endif // OPENVAC_CELLDATA_H
