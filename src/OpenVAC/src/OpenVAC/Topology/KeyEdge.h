// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_KEYEDGE_H
#define OPENVAC_KEYEDGE_H

#include <OpenVAC/Topology/KeyCell.h>
#include <OpenVAC/Topology/EdgeCell.h>

namespace OpenVAC
{

class KeyEdge: public KeyCell, public EdgeCell
{
public:
    // Constructor
    KeyEdge(VAC * vac, CellId id, const KeyEdgeData & data = KeyEdgeData());

    // Cell type
    CellType type() const { return CellType::KeyEdge; }

    // Cell data
    const KeyEdgeData & data() const;

    // Frame
    Frame frame() const;

    // Start vertex
    KeyVertexHandle startVertex() const;

    // End vertex
    KeyVertexHandle endVertex() const;

private:
    // Data
    KeyEdgeData data_;
    KeyEdgeData & data();

    // Befriend Operator
    friend class Operator;

    // Casting
    OPENVAC_DEFINE_CELL_CAST(KeyEdge)
};

}

#endif // OPENVAC_KEYEDGE_H
