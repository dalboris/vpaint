// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_VAC_H
#define OPENVAC_VAC_H

#include <OpenVAC/Core/IdManager.h>
#include <OpenVAC/Topology/Cell.h>

// Declare VAC public typedefs

#define OPENVAC_VAC_DECLARE_GEOMETRY_TYPEDEF_ \
    typedef Geometry Geometry_t;

#define OPENVAC_VAC_DECLARE_OPERATOR_TYPEDEF_ \
    typedef OpenVAC::Operator<Geometry> Operator;

#define OPENVAC_VAC_DECLARE_HANDLE_TYPEDEF_(CellType) \
    typedef OpenVAC::CellType##Handle<Geometry> CellType##Handle;

#define OPENVAC_VAC_DECLARE_DATA_TYPEDEF_(CellType) \
    typedef OpenVAC::CellType##Data<Geometry> CellType##Data;

#define OPENVAC_VAC_DECLARE_TYPEDEFS \
    OPENVAC_VAC_DECLARE_OPERATOR_TYPEDEF_ \
    OPENVAC_FOREACH_CELL_TYPE(OPENVAC_VAC_DECLARE_HANDLE_TYPEDEF_) \
    OPENVAC_FOREACH_CELL_DATA_TYPE(OPENVAC_VAC_DECLARE_DATA_TYPEDEF_)

// Using VAC types within current scope

#define OPENVAC_USING_CELLTYPE \
    typedef OpenVAC::CellType CellType;

#define OPENVAC_USING_VAC_OPERATOR(Vac) \
    typedef Vac::Operator Operator;

#define OPENVAC_USING_VAC_CELL_HANDLE(CellType, Vac) \
    typedef Vac::CellType##Handle CellType##Handle;

#define OPENVAC_USING_VAC_CELL_ID(CellType, Vac) \
    typedef OpenVAC::CellType##Id CellType##Id;

#define OPENVAC_USING_VAC_CELL_DATA(CellType, Vac) \
    typedef Vac::CellType##Data CellType##Data;

#define OPENVAC_USING_VAC_TYPES(Vac) \
    OPENVAC_USING_CELLTYPE \
    OPENVAC_USING_VAC_OPERATOR(Vac) \
    OPENVAC_FOREACH_CELL_TYPE_ARGS(OPENVAC_USING_VAC_CELL_HANDLE, Vac) \
    OPENVAC_FOREACH_CELL_TYPE_ARGS(OPENVAC_USING_VAC_CELL_ID, Vac) \
    OPENVAC_FOREACH_CELL_DATA_TYPE_ARGS(OPENVAC_USING_VAC_CELL_DATA, Vac)

namespace OpenVAC
{

template <class Geometry>
class VAC
{
private:
    // Private typedefs
    typedef OpenVAC::CellSharedPtr<Geometry>  CellSharedPtr;
    typedef OpenVAC::IdManager<CellSharedPtr> CellManager;
    typedef typename Geometry::Manager        GeometryManager;

public:
    // Public typedefs
    OPENVAC_VAC_DECLARE_TYPEDEFS

    // Construct a VAC.
    VAC() : cellManager_(), geometryManager_() {}

    // Number of cells
    size_t numCells() const { return cellManager_.size(); }

    // Get cell from ID
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

}

#endif // OPENVAC_ VAC_H
