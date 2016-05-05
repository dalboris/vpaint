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
#include <OpenVac/Geometry.h>

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
class Vac
{
public:
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

    // XXX create CellHandleVector classes
    std::vector<CellHandle> cells() const
    {
        std::vector<CellHandle> res;
        for (const auto & pair: cellManager_)
        {
            const SharedPtr<Cell> & sp = pair.second;
            res.push_back(CellHandle(sp));
        }
        return res;
    }

private:
    // Cell manager
    IdManager<SharedPtr<Cell>> cellManager_;

    // Geomety manager
    Geometry::GeometryManager geometryManager_;

    // Befriend Operator
    friend class Operator;
};

} // end namespace OpenVac

#endif // OPENVAC_ VAC_H
