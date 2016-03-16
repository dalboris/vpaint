// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_KEYCELL_H
#define OPENVAC_KEYCELL_H

#include <OpenVac/Topology/Cell.h>
#include <OpenVac/Geometry.h>

namespace OpenVac
{

/// \class KeyCell OpenVac/Topology/KeyCell.h
/// \brief Virtual base class of KeyVertex, KeyEdge, and KeyFace
///
class KeyCell: virtual public Cell
{
public:
    /// Constructs a KeyCell.
    KeyCell(Vac * vac, CellId id) : Cell(vac, id) {}

    /// Destruct an EdgeCell.
    virtual ~KeyCell() {}

    /// Accesses the Frame of this KeyCell.
    virtual const Geometry::Frame & frame() const=0;

private:
    // Befriend Operator
    friend class Operator;

    // Casting
    OPENVAC_CELL_DEFINE_CAST_(KeyCell)
};

} // end namespace OpenVac

#endif // OPENVAC_KEYCELL_H
