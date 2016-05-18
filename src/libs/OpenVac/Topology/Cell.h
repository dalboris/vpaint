// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_CELL_H
#define OPENVAC_CELL_H

#include <OpenVac/Core/CellId.h>
#include <OpenVac/Core/CellType.h>
#include <OpenVac/Core/CellIdType.h>
#include <OpenVac/Data/CellData.h>
#include <OpenVac/Topology/Util/Handles.h>
#include <OpenVac/Topology/Util/CellCast.h>

namespace OpenVac
{

class Vac;
class Operator;

/// \class Cell OpenVac/Topology/Cell.h
/// \brief Virtual base class of all cell classes
///
class Cell
{
public:
    /// Constructs a Cell.
    Cell(Vac * vac, CellId id) : vac_(vac), id_(id) {}

    /// Destructs a Cell.
    virtual ~Cell() {}

    /// Returns the derived type of a Cell.
    virtual CellType type() const=0;

    /// Returns the Vac that this cell belongs to.
    Vac * vac() const { return vac_; }

    /// Returns the ID of this Cell. Cannot be 0, reserved for "no cell".
    CellId id() const { return id_; }

    /// Returns the ID and type of this Cell.
    CellIdType idType() const { return CellIdType(id(), type()); }

    /// Access the data of this Cell.
    virtual const CellData<Handles> & data() const=0;

private:
    // Member variables
    Vac * vac_;
    CellId id_;

    // Non-const data access
    virtual CellData<Handles> & data()=0;

    // Befriend Operator
    friend class Operator;

    // Casting
    OPENVAC_CELL_DEFINE_CAST_BASE_
    OPENVAC_CELL_DEFINE_CAST_(Cell)
};

} // end namespace OpenVac

#endif // OPENVAC_CELL_H
