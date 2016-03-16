// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_VERTEXCELL_H
#define OPENVAC_VERTEXCELL_H

#include <OpenVac/Topology/Cell.h>

namespace OpenVac
{

/// \class VertexCell OpenVac/Topology/VertexCell.h
/// \brief Virtual base class of KeyVertex and InbetweenVertex
///
class VertexCell: virtual public Cell
{
public:
    /// Constructors a VertexCell.
    VertexCell(Vac * vac, CellId id) : Cell(vac, id) {}

    /// Destructs a VertexCell.
    virtual ~VertexCell() {}

private:
    // Befriend Operator
    friend class Operator;

    // Casting
    OPENVAC_CELL_DEFINE_CAST_(VertexCell)
};

} // end namespace OpenVac

#endif // OPENVAC_VERTEXCELL_H
