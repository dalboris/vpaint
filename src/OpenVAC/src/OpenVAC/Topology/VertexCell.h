// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_VERTEXCELL_H
#define OPENVAC_VERTEXCELL_H

#include <OpenVAC/Topology/Cell.h>

namespace OpenVAC
{

class VertexCell: virtual public Cell
{
public:
    // Constructor
    VertexCell(VAC * vac, CellId id);

    // Virtual destructor
    virtual ~VertexCell()=0;

private:
    // Befriend Operator
    friend class Operator;

    // Casting
    OPENVAC_DEFINE_CELL_CAST(VertexCell)
};

}

#endif // OPENVAC_VERTEXCELL_H
