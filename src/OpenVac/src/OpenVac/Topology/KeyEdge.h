// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_KEYEDGE_H
#define OPENVAC_KEYEDGE_H

#include <OpenVac/Topology/KeyCell.h>
#include <OpenVac/Topology/EdgeCell.h>

namespace OpenVac
{

/// \class KeyEdge OpenVac/Topology/KeyEdge.h
/// \brief A class that represents a key edge
///
template <class Geometry>
class KeyEdge: public KeyCell<Geometry>, public EdgeCell<Geometry>
{
public:
    // Typedefs
    OPENVAC_CELL_DECLARE_TYPE_ALIASES_

    // Constructor
    KeyEdge(Vac * vac, CellId id, const KeyEdgeData & data = KeyEdgeData()) :
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
    const KeyVertexHandle & startVertex() const { return data().startVertex; }

    // End vertex
    const KeyVertexHandle & endVertex() const { return data().endVertex; }

private:
    // Data
    KeyEdgeData data_;

    // Non-const data access
    CellData & data() { return data_; }

    // Befriend Operator
    friend Operator;

    // Casting
    OPENVAC_DEFINE_CELL_CAST(KeyEdge)
};

} // end namespace OpenVac

#endif // OPENVAC_KEYEDGE_H
