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

template <class Geometry>
class KeyEdge: public KeyCell<Geometry>, public EdgeCell<Geometry>
{
public:
    // Typedefs
    OPENVAC_CELL_DECLARE_TYPEDEFS(KeyEdge)

    // Constructor
    KeyEdge(VAC * vac, CellId id, const KeyEdgeData & data = KeyEdgeData()) :
        Cell<Geometry>(vac, id),
        KeyCell<Geometry>(vac, id),
        EdgeCell<Geometry>(vac, id),
        data_(data) {}

    // Cell type
    CellType type() const { return CellType::KeyEdge; }

    // Cell data
    const KeyEdgeData & data() const { return data_; }

    // Frame
    Frame frame() const { return data().frame; }

    // Start vertex
    KeyVertexHandle startVertex() const { return data().startVertex; }

    // End vertex
    KeyVertexHandle endVertex() const { return data().endVertex; }

private:
    // Data
    KeyEdgeData data_;
    KeyEdgeData & data() { return data_; }

    // Befriend Operator
    friend Operator;

    // Casting
    OPENVAC_DEFINE_CELL_CAST(KeyEdge)
};

}

#endif // OPENVAC_KEYEDGE_H
