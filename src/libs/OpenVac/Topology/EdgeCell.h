// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_EDGECELL_H
#define OPENVAC_EDGECELL_H

#include <OpenVac/Topology/Cell.h>

namespace OpenVac
{

/// \class EdgeCell OpenVac/Topology/EdgeCell.h
/// \brief Virtual base class of KeyEdge and InbetweenEdge
///
class EdgeCell: virtual public Cell
{
public:
    /// Constructs an EdgeCell.
    EdgeCell(Vac * vac, CellId id) : Cell(vac, id) {}

    /// Destruct an EdgeCell.
    virtual ~EdgeCell() {}

private:
    // Befriend Operator
    friend class Operator;

    // Casting
    OPENVAC_CELL_DEFINE_CAST_(EdgeCell)
};

} // end namespace OpenVac

#endif // OPENVAC_EDGECELL_H
