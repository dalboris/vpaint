// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_VAC_H
#define OPENVAC_VAC_H

#include <OpenVac/Core/IdManager.h>
#include <OpenVac/Data/UsingData.h>
#include <OpenVac/Topology/Cell.h>

/************** Private macros to declare Vac type aliases  ******************/

// public type aliases

#define OPENVAC_VAC_USING_GEOMETRY_ \
    using geometry_type = Geometry;

#define OPENVAC_VAC_USING_DATA_ \
    OPENVAC_USING_DATA(/*  No prefix  */, UsingCellHandlesAsCellRefs<Geometry>, Geometry);

#define OPENVAC_VAC_USING_OPERATOR_ \
    using Operator = OpenVac::Operator<Geometry>;

#define OPENVAC_VAC_USING_CELL_HANDLE_(CellType) \
    using CellType##Handle = OpenVac::Handle< CellType<Geometry> >;

#define OPENVAC_VAC_USING_CELL_ID_(CellType) \
    using CellType##Id = OpenVac::CellType##Id;

#define OPENVAC_VAC_DECLARE_PUBLIC_TYPE_ALIASES_ \
    OPENVAC_VAC_USING_GEOMETRY_ \
    OPENVAC_VAC_USING_DATA_ \
    OPENVAC_VAC_USING_OPERATOR_ \
    OPENVAC_FOREACH_CELL_TYPE(OPENVAC_VAC_USING_CELL_HANDLE_) \
    OPENVAC_FOREACH_CELL_TYPE(OPENVAC_VAC_USING_CELL_ID_) \

// private type aliases

#define OPENVAC_VAC_DECLARE_PRIVATE_TYPE_ALIASES_ \
    using CellSharedPtr = OpenVac::SharedPtr< Cell<Geometry> >; \
    using CellManager = OpenVac::IdManager< CellSharedPtr >; \
    using GeometryManager = typename Geometry::Manager;

/// \namespace OpenVac
/// \brief The OpenVAC library
///
/// OpenVAC is an open-source C++ implementation of the Vector Animation
/// Complex (VAC), See \ref mainpage "Getting started" for more details.
///
namespace OpenVac
{

/********************************* Vac  **************************************/

/// \class Vac OpenVac/Vac.h
/// \brief A class to represent a Vector Animation Complex
///
template <class Geometry>
class Vac
{
private:
    OPENVAC_VAC_DECLARE_PRIVATE_TYPE_ALIASES_

public:
    OPENVAC_VAC_DECLARE_PUBLIC_TYPE_ALIASES_

    /// Constructs a Vac.
    Vac() : cellManager_(), geometryManager_() {}

    /// Returns the number of cells in the Vac.
    size_t numCells() const { return cellManager_.size(); }

    /// Returns a handle to the cell with the given \p id. Returns an empty
    /// handle if no cell has the given \p id.
    CellHandle cell(CellId id) const
    {
        if (cellManager_.contains(id))
            return cellManager_[id];
        else
            return CellHandle();
    }

private:
    // Cell manager
    CellManager cellManager_;

    // Geomety manager
    GeometryManager geometryManager_;

    // Befriend Operator
    friend Operator;
};

} // end namespace OpenVac

#endif // OPENVAC_ VAC_H
