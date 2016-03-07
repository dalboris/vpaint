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
#include <OpenVac/Topology/Cell.h>

#define OPENVAC_VAC_DECLARE_GEOMETRY_TYPE_ \
    typedef Geometry geometry_type;

#define OPENVAC_VAC_DECLARE_VAC_PTRS_ \
    typedef OpenVac::WeakPtr<Vac> Ptr; \
    typedef OpenVac::WeakPtr<Vac> VacPtr; \
    typedef OpenVac::SharedPtr<Vac> SharedPtr; \
    typedef OpenVac::SharedPtr<Vac> VacSharedPtr;

#define OPENVAC_VAC_DECLARE_OPERATOR_TYPEDEF_ \
    typedef OpenVac::Operator<Geometry> Operator;

#define OPENVAC_VAC_DECLARE_HANDLE_TYPEDEF_(CellType) \
    typedef OpenVac::CellType##Handle<Geometry> CellType##Handle;

#define OPENVAC_VAC_DECLARE_ID_TYPEDEF_(CellType) \
    typedef OpenVac::CellType##Id CellType##Id;

#define OPENVAC_VAC_DECLARE_DATA_TYPEDEF_(CellType) \
    typedef OpenVac::CellType##Data<Geometry> CellType##Data;

#define OPENVAC_VAC_DECLARE_TYPEDEFS \
    OPENVAC_VAC_DECLARE_GEOMETRY_TYPE_ \
    OPENVAC_VAC_DECLARE_VAC_PTRS_ \
    OPENVAC_VAC_DECLARE_OPERATOR_TYPEDEF_ \
    OPENVAC_FOREACH_CELL_TYPE(OPENVAC_VAC_DECLARE_HANDLE_TYPEDEF_) \
    OPENVAC_FOREACH_CELL_TYPE(OPENVAC_VAC_DECLARE_ID_TYPEDEF_) \
    OPENVAC_FOREACH_CELL_DATA_TYPE(OPENVAC_VAC_DECLARE_DATA_TYPEDEF_)

/// \namespace OpenVac
/// \brief The OpenVAC library
///
/// OpenVAC is an open-source C++ implementation of the Vector Animation
/// Complex (VAC), See \ref mainpage "Getting started" for more details.
///
namespace OpenVac
{

template <class Geometry>
class Vac
{
private:
    // Private typedefs
    typedef OpenVac::CellSharedPtr<Geometry>  CellSharedPtr;
    typedef OpenVac::IdManager<CellSharedPtr> CellManager;
    typedef typename Geometry::Manager        GeometryManager;

public:
    // Public typedefs
    OPENVAC_VAC_DECLARE_TYPEDEFS

    // Construct a VAC.
    Vac() : cellManager_(), geometryManager_(), ptrProvider_(this) {}

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

    // Returns a weak pointer to this vac
    Ptr & ptr() { return ptrProvider_.ptr(); }

    // Construct a Vac managed via a shared pointer
    static SharedPtr make_shared()
    {
        auto vac = std::make_shared<Vac>();
        vac->ptrProvider_.setShared(vac);
        return vac;
    }

private:
    // Cell manager
    CellManager cellManager_;

    // Geomety manager
    GeometryManager geometryManager_;

    // Weak pointer provider
    WeakPtrProvider<Vac> ptrProvider_;

    // Befriend Operator
    friend Operator;
};

}

#endif // OPENVAC_ VAC_H
