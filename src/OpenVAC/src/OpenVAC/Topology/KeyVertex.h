// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_KEYVERTEX_H
#define OPENVAC_KEYVERTEX_H

#include <OpenVAC/Topology/KeyCell.h>
#include <OpenVAC/Topology/VertexCell.h>

namespace OpenVAC
{

class KeyVertex: public KeyCell, public VertexCell
{
public:
    // Constructor
    KeyVertex(VAC * vac, CellId id, const KeyVertexData & data);

    // Cell type
    CellType type() const { return CellType::KeyVertex; }

    // Cell data
    const KeyVertexData & data() const;

    // Frame
    Frame frame() const;

private:
    // Data
    KeyVertexData data_;

    // Befriend Operator
    friend class Operator;

    // Casting
    OPENVAC_DEFINE_CELL_CAST(KeyVertex)
};

}

#endif // OPENVAC_KEYVERTEX_H
