// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_CELL_H
#define OPENVAC_CELL_H

#include <OpenVAC/Topology/CellId.h>
#include <OpenVAC/Topology/CellData.h>

namespace OpenVAC
{

class VAC;

class Cell
{
public:
    // Constructor
    Cell(VAC * vac, CellId id);

    // Virtual destructor
    virtual ~Cell()=0;

    // Type
    virtual CellType type() const=0;

    // VAC this cell belongs to
    VAC * vac() const;

    // Cell id
    CellId id() const;

    // Cell data
    virtual const CellData & data() const=0;

private:
    // Member variables
    VAC * vac_;
    CellId id_;

    // Befriend Operator
    friend class Operator;

    // Casting
    OPENVAC_DEFINE_CELL_CAST_BASE
    OPENVAC_DEFINE_CELL_CAST(Cell)
};

}

#endif // OPENVAC_CELL_H
