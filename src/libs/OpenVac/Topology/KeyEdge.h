// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_KEYEDGE_H
#define OPENVAC_KEYEDGE_H

#include <OpenVac/Data/KeyEdgeData.h>
#include <OpenVac/Topology/KeyCell.h>
#include <OpenVac/Topology/EdgeCell.h>

namespace OpenVac
{

/// \class KeyEdge OpenVac/Topology/KeyEdge.h
/// \brief A class that represents a key edge
///
class KeyEdge: public KeyCell, public EdgeCell
{
public:
    /// Constructs a KeyEdge.
    KeyEdge(Vac * vac, CellId id, const KeyEdgeData<Handles> & data = KeyEdgeData<Handles>()) :
        Cell(vac, id),
        KeyCell(vac, id),
        EdgeCell(vac, id),
        data_(data) {}

    /// Returns the type of this cell, i.e. CellType::KeyEdge.
    CellType type() const { return CellType::KeyEdge; }

    /// Accesses the KeyEdgeData of this KeyEdge.
    const KeyEdgeData<Handles> & data() const { return data_; }

    /// Accesses the start vertex of this KeyEdge.
    const KeyVertexHandle & startVertex() const { return data().startVertex; }

    /// Accesses the end vertex of this KeyEdge.
    const KeyVertexHandle & endVertex() const { return data().endVertex; }

    /// Accesses the Frame of this KeyEdge.
    const Geometry::Frame & frame() const { return data().frame; }

    /// Accesses the KeyEdgeGeometry of this KeyEdge.
    const Geometry::KeyEdgeGeometry & geometry() const { return data().geometry; }

    /// Accesses and allows modification of the KeyEdgeGeometry of this KeyEdge.
    Geometry::KeyEdgeGeometry & geometry() { return data().geometry; }

private:
    // Data
    KeyEdgeData<Handles> data_;

    // Non-const data access
    KeyEdgeData<Handles> & data() { return data_; }

    // Befriend Operator
    friend class Operator;

    // Casting
    OPENVAC_CELL_DEFINE_CAST_(KeyEdge)
};

} // end namespace OpenVac

#endif // OPENVAC_KEYEDGE_H
