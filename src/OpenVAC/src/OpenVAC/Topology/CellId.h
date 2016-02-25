// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_CELL_ID_H
#define OPENVAC_CELL_ID_H

namespace OpenVAC
{

#define OPENVAC_DECLARE_CELL_ID(CellType) \
    typedef unsigned int CellType##Id;

OPENVAC_DECLARE_CELL_ID(Cell)
OPENVAC_DECLARE_CELL_ID(KeyCell)
OPENVAC_DECLARE_CELL_ID(InbetweenCell)
OPENVAC_DECLARE_CELL_ID(VertexCell)
OPENVAC_DECLARE_CELL_ID(EdgeCell)
OPENVAC_DECLARE_CELL_ID(FaceCell)
OPENVAC_DECLARE_CELL_ID(KeyVertex)
OPENVAC_DECLARE_CELL_ID(KeyEdge)
OPENVAC_DECLARE_CELL_ID(KeyFace)
OPENVAC_DECLARE_CELL_ID(InbetweenVertex)
OPENVAC_DECLARE_CELL_ID(InbetweenEdge)
OPENVAC_DECLARE_CELL_ID(InbetweenFace)

}

#endif // OPENVAC_CELL_ID_H
