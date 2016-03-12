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
template <class Geometry>
class VertexCell: virtual public Cell<Geometry>
{
public:
    // Typedefs
    OPENVAC_CELL_DECLARE_TYPE_ALIASES_

    // Constructor
    VertexCell(Vac * vac, CellId id) : Cell<Geometry>(vac, id) {}

    // Virtual destructor
    virtual ~VertexCell() {}

private:
    // Befriend Operator
    friend Operator;

    // Casting
    OPENVAC_DEFINE_CELL_CAST(VertexCell)
};

} // end namespace OpenVac

#endif // OPENVAC_VERTEXCELL_H
