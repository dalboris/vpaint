// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_EDGECELL_H
#define OPENVAC_EDGECELL_H

#include <OpenVAC/Topology/Cell.h>

namespace OpenVAC
{

class EdgeCell: virtual public Cell
{
public:
    // Constructor
    EdgeCell(VAC * vac, CellId id);

    // Virtual destructor
    virtual ~EdgeCell()=0;

private:
    // Befriend Operator
    friend class Operator;

    // Casting
    OPENVAC_DEFINE_CELL_CAST(EdgeCell)
};

}

#endif // OPENVAC_EDGECELL_H
