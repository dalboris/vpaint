// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_DEFINECELLDATA_H
#define OPENVAC_DEFINECELLDATA_H

#include <OpenVAC/Topology/TCellData/TCellData.h>
#include <OpenVAC/Topology/TCellData/TKeyVertexData.h>
#include <OpenVAC/Topology/TCellData/TKeyEdgeData.h>

#define OPENVAC_DEFINE_CELLTYPE_DATA(Prefix, CellType, CellDataTrait) \
    typedef OpenVAC::T##CellType##Data<CellDataTrait> Prefix##CellType##Data;

#define OPENVAC_DEFINE_CELL_DATA(Prefix, CellDataTrait) \
    OPENVAC_DEFINE_CELLTYPE_DATA(Prefix, Cell, CellDataTrait) \
    OPENVAC_DEFINE_CELLTYPE_DATA(Prefix, KeyVertex, CellDataTrait) \
    OPENVAC_DEFINE_CELLTYPE_DATA(Prefix, KeyEdge, CellDataTrait)

#endif // OPENVAC_DEFINECELLDATA_H
