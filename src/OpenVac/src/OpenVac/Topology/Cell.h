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
#include <OpenVac/Core/Memory.h>
#include <OpenVac/Data/UsingData.h>
#include <OpenVac/Geometry/UsingGeometry.h>


/************** Private macros to declare Cell type aliases  *****************/

#define OPENVAC_CELL_USING_GEOMETRY_ \
    using geometry_type = Geometry; \
    OPENVAC_USING_GEOMETRY(/* No prefix */, Geometry)

#define OPENVAC_CELL_USING_VAC_ \
    using Vac = OpenVac::Vac<Geometry>;

#define OPENVAC_CELL_USING_DATA_ \
    OPENVAC_USING_DATA(/* No prefix */, UsingCellHandlesAsCellRefs<Geometry>, Geometry);

#define OPENVAC_CELL_USING_OPERATOR_ \
    using Operator = OpenVac::Operator<Geometry>;

#define OPENVAC_CELL_USING_CELL_HANDLE_(CellType) \
    using CellType##Handle = OpenVac::Handle<CellType<Geometry>>;

#define OPENVAC_CELL_DECLARE_TYPE_ALIASES_ \
    OPENVAC_CELL_USING_GEOMETRY_ \
    OPENVAC_CELL_USING_VAC_ \
    OPENVAC_CELL_USING_DATA_ \
    OPENVAC_CELL_USING_OPERATOR_ \
    OPENVAC_FOREACH_CELL_TYPE(OPENVAC_CELL_USING_CELL_HANDLE_)

/********** Private macros to define handle_cast member functions  ***********/

#define OPENVAC_DEFINE_STATIC_TO_CELL_(CellType) \
    static CellType<Geometry> * to##CellType(Cell * c) \
    { \
        return c ? c->to##CellType##_() : nullptr; \
    }

#define OPENVAC_DEFINE_MEMBER_TO_CELL_(CellType) \
    virtual CellType<Geometry> * to##CellType##_() \
    { \
        return nullptr; \
    }

#define OPENVAC_DEFINE_CELL_CAST_BASE \
    protected: \
        OPENVAC_FOREACH_CELL_TYPE(OPENVAC_DEFINE_STATIC_TO_CELL_) \
    private: \
        OPENVAC_FOREACH_DERIVED_CELL_TYPE(OPENVAC_DEFINE_MEMBER_TO_CELL_)

#define OPENVAC_DEFINE_CELL_CAST(CellType) \
    template<class T, class U> \
    friend Handle<T> handle_cast(const Handle<U> & r); \
    \
    template<class T, class U> \
    friend Handle<T> handle_cast(const SharedPtr<U> & r); \
    \
    CellType<Geometry> * to##CellType##_() \
    { \
        return this; \
    } \
    \
    static CellType<Geometry> * handle_cast(Cell<Geometry> * c) \
    { \
        return Cell<Geometry>::to##CellType(c); \
    }


namespace OpenVac
{

/*********************** Forward declare Cell classes ************************/

#define OPENVAC_CELL_FORWARD_DECLARE_CELL_(CellType) \
    template <class Geometry> \
    class CellType;

OPENVAC_FOREACH_CELL_TYPE(OPENVAC_CELL_FORWARD_DECLARE_CELL_)


/************************* UsingCellHandlesAsCellRefs  ***********************/

/// \class UsingCellHandlesAsCellRefs OpenVac/Topology/Cell.h
/// \brief A class that declares CellType##Ref as an alias for
/// Handle<CellType>, for each cell type.
///
/// UsingCellHandlesAsCellRefs is used as the T template argument of the Data
/// classes used within the Cell classes.
///
template <class Geometry>
class UsingCellHandlesAsCellRefs
{
#define OPENVAC_CELL_USING_HANDLE_AS_REF_(CellType) \
    using CellType##Ref = Handle<CellType<Geometry>>;

public:
    OPENVAC_FOREACH_CELL_TYPE(OPENVAC_CELL_USING_HANDLE_AS_REF_)
};


/*********************************** Cell ************************************/

template <class Geometry> class Operator;
template <class Geometry> class Vac;

/// \class Cell OpenVac/Topology/Cell.h
/// \brief Virtual base class of all cell classes
///
template <class Geometry>
class Cell
{
public:
    // Type aliases
    OPENVAC_CELL_DECLARE_TYPE_ALIASES_

    // Constructor
    Cell(Vac * vac, CellId id) : vac_(vac), id_(id) {}

    // Virtual destructor
    virtual ~Cell() {}

    // Type
    virtual CellType type() const=0;

    // VAC this cell belongs to
    Vac * vac() const { return vac_; }

    // Cell id
    CellId id() const { return id_; }

    // Cell data
    virtual const CellData & data() const=0;

private:
    // Member variables
    Vac * vac_;
    CellId id_;

    // Non-const data access
    virtual CellData & data()=0;

    // Befriend Operator
    friend Operator;

    // Casting
    OPENVAC_DEFINE_CELL_CAST_BASE
    OPENVAC_DEFINE_CELL_CAST(Cell)
};

} // end namespace OpenVac

#endif // OPENVAC_CELL_H
