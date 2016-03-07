// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_CELL_H
#define OPENVAC_CELL_H

#include <OpenVac/Topology/CellId.h>
#include <OpenVac/Topology/CellData.h>

#define OPENVAC_CELL_DECLARE_VAC_TYPEDEF_ \
    typedef OpenVac::Vac<Geometry> Vac; \
    typedef OpenVac::WeakPtr<Vac> VacPtr;

#define OPENVAC_CELL_DECLARE_OPERATOR_TYPEDEF_ \
    typedef OpenVac::Operator<Geometry> Operator;

#define OPENVAC_CELL_DECLARE_HANDLE_TYPEDEF_(CellType) \
    typedef OpenVac::CellType##Handle<Geometry> CellType##Handle;

#define OPENVAC_CELL_DECLARE_DATA_TYPEDEF_(CellType) \
    typedef OpenVac::CellType##Data<Geometry> CellType##Data;

#define OPENVAC_CELL_DECLARE_TYPEDEFS(CellType) \
    OPENVAC_CELL_DECLARE_VAC_TYPEDEF_ \
    OPENVAC_CELL_DECLARE_OPERATOR_TYPEDEF_ \
    OPENVAC_FOREACH_CELL_TYPE(OPENVAC_CELL_DECLARE_HANDLE_TYPEDEF_) \
    OPENVAC_FOREACH_CELL_DATA_TYPE(OPENVAC_CELL_DECLARE_DATA_TYPEDEF_)

#include <iostream>

namespace OpenVac
{

template <class Geometry> class Operator;
template <class Geometry> class Vac;

template <class Geometry>
class Cell
{
public:
    // Typedefs
    OPENVAC_CELL_DECLARE_TYPEDEFS(Cell)

    // Constructor
    Cell(VacPtr vac, CellId id) : vac_(vac), id_(id) {}

    // Virtual destructor
    virtual ~Cell() {}

    // Type
    virtual CellType type() const=0;

    // VAC this cell belongs to
    VacPtr vac() const { return vac_; }

    // Cell id
    CellId id() const { return id_; }

    // Cell data
    virtual const CellData & data() const=0;

private:
    // Member variables
    VacPtr vac_;
    CellId id_;

    // Non-const data access
    virtual CellData & data()=0;

    // Befriend Operator
    friend Operator;

    // Casting
    OPENVAC_DEFINE_CELL_CAST_BASE
    OPENVAC_DEFINE_CELL_CAST(Cell)
};

}

#endif // OPENVAC_CELL_H
