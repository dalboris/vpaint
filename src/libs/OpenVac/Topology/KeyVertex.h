// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_KEYVERTEX_H
#define OPENVAC_KEYVERTEX_H

#include <OpenVac/Data/KeyVertexData.h>
#include <OpenVac/Topology/KeyCell.h>
#include <OpenVac/Topology/VertexCell.h>

namespace OpenVac
{

/// \class KeyVertex OpenVac/Topology/KeyVertex.h
/// \brief A class that represents a key vertex
///
class KeyVertex: public KeyCell, public VertexCell
{
public:
    /// Constructs a KeyVertex.
    KeyVertex(Vac * vac, CellId id, const KeyVertexData<Handles> & data = KeyVertexData<Handles>()) :
        Cell(vac, id),
        KeyCell(vac, id),
        VertexCell(vac, id),
        data_(data) {}

    /// Returns the type of this cell, i.e. CellType::KeyVertex.
    CellType type() const { return CellType::KeyVertex; }

    /// Accesses the KeyVertexData of this KeyVertex.
    const KeyVertexData<Handles> & data() const { return data_; }

    /// Accesses the Frame of this KeyVertex.
    const Geometry::Frame & frame() const { return data().frame; }

    /// Accesses the KeyVertexGeometry of this KeyVertex.
    const Geometry::KeyVertexGeometry & geometry() const { return data().geometry; }

private:
    // Data
    KeyVertexData<Handles> data_;

    // Non-const data access
    KeyVertexData<Handles> & data() { return data_; }

    // Befriend Operator
    friend class Operator;

    // Casting
    OPENVAC_CELL_DEFINE_CAST_(KeyVertex)
};

} // end namespace OpenVac

#endif // OPENVAC_KEYVERTEX_H
