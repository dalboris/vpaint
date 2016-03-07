// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_CELLTYPE_H
#define OPENVAC_CELLTYPE_H

namespace OpenVac
{

enum class CellType : char
{
    Cell       = 0x00,

    KeyCell       = 0x01,
    InbetweenCell = 0x02,

    VertexCell    = 0x10,
    EdgeCell      = 0x20,
    FaceCell      = 0x40,

    KeyVertex = KeyCell | VertexCell,
    KeyEdge   = KeyCell | EdgeCell,
    KeyFace   = KeyCell | FaceCell,

    InbetweenVertex = InbetweenCell | VertexCell,
    InbetweenEdge   = InbetweenCell | EdgeCell,
    InbetweenFace   = InbetweenCell | FaceCell
};

inline constexpr CellType operator&(CellType t1, CellType t2)
{
    return static_cast<CellType> (static_cast<char>(t1) & static_cast<char>(t2));
}

}

#endif // OPENVAC_CELLTYPE_H
