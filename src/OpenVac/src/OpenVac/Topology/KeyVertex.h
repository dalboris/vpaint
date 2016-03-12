// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_KEYVERTEX_H
#define OPENVAC_KEYVERTEX_H

#include <OpenVac/Topology/KeyCell.h>
#include <OpenVac/Topology/VertexCell.h>

namespace OpenVac
{

/// \class KeyVertex OpenVac/Topology/KeyVertex.h
/// \brief A class that represents a key vertex
///
template <class Geometry>
class KeyVertex: public KeyCell<Geometry>, public VertexCell<Geometry>
{
public:
    // Typedefs
    OPENVAC_CELL_DECLARE_TYPE_ALIASES_

    // Constructor
    KeyVertex(Vac * vac, CellId id, const KeyVertexData & data = KeyVertexData()) :
        Cell<Geometry>(vac, id),
        KeyCell<Geometry>(vac, id),
        VertexCell<Geometry>(vac, id),
        data_(data) {}

    // Cell type
    CellType type() const { return CellType::KeyVertex; }

    // Cell data
    const KeyVertexData & data() const { return data_; }

    // Frame
    Frame frame() const { return data().frame; }

private:
    // Data
    KeyVertexData data_;

    // Non-const data access
    CellData & data() { return data_; }

    // Befriend Operator
    friend Operator;

    // Casting
    OPENVAC_DEFINE_CELL_CAST(KeyVertex)
};

} // end namespace OpenVac

#endif // OPENVAC_KEYVERTEX_H
