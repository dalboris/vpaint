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

namespace OpenVac
{

template <class Geometry>
class KeyCell: virtual public Cell<Geometry>
{
public:
    // Typedefs
    OPENVAC_CELL_DECLARE_TYPEDEFS(KeyCell)

    // Constructor
    KeyCell(VacPtr vac, CellId id) : Cell<Geometry>(vac, id) {}

    // Virtual destructor
    virtual ~KeyCell() {}

    // Frame
    virtual typename Geometry::Frame frame() const=0;

private:
    // Befriend Operator
    friend Operator;

    // Casting
    OPENVAC_DEFINE_CELL_CAST(KeyCell)
};

}

#endif // OPENVAC_KEYCELL_H
