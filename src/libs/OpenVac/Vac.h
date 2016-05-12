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
#include <OpenVac/Topology/Util/CellHandleSet.h>
#include <OpenVac/Util/VacObserver.h>
#include <OpenVac/Geometry.h>

#include <unordered_set>

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
///
class Vac
{
public:
    /// Constructs a Vac.
    ///
    Vac() : cellManager_(), geometryManager_(), areTopologyEditsConcatenated_(false) {}

    /// Returns the number of cells in the Vac.
    ///
    size_t numCells() const { return cellManager_.size(); }

    /// Returns a handle to the cell with the given \p id. Returns an empty
    /// handle if no cell has the given \p id.
    ///
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

    /// Registers an observer.
    ///
    void registerObserver(VacObserver * observer)
    {
        observers_.insert(observer);
    }

    /// Unregisters an observer.
    ///
    void unregisterObserver(VacObserver * observer)
    {
        observers_.erase(observer);
    }

    /// Clients may call beginTopologyEdit() whenever they are about to edit the
    /// topology of cells (i.e., using an Operator), but it is not required
    /// to do so.
    ///
    /// Calling this method will merge all topologyChanged() notifications that
    /// should have been called between beginTopologyEdit() and
    /// endTopologyEdit(), into a single notification called in
    /// endTopologyEdit(). Use this whenever you are planning to use two or
    /// more topological operators in a row, but would like only one
    /// topologyChanged() notification to be sent.
    ///
    void beginTopologyEdit()
    {
        areTopologyEditsConcatenated_ = true;
        topologyEditCreated_.clear();
        topologyEditDestroyed_.clear();
        topologyEditAffected_.clear();
    }

    /// When clients choose to call beginTopologyEdit(), then they must call
    /// endTopologyEdit() when they are done editing the topology.
    ///
    void endTopologyEdit()
    {
        emitTopologyChanged_();
    }

    /// Clients must call this method whenever they are about to edit the
    /// geometry of cells. It must not be called if beginGeometryEdit() was
    /// previously called without having yet called its corresponding
    /// endGeometryEdit().
    ///
    /// One may ask: why this design choice? Indeed, another option would have
    /// been to make Cell::geometry() return a const reference (instead of a
    /// mutable reference), and have a function Cell::setGeometry() that
    /// ensures the notification gets sent. The rationale against is option is
    /// that users of OpenVac may define arbitrarily complex geometry, and
    /// therefore, e.g., InbetweenFaceGeometry might be extremely expensive to
    /// copy. Allowing clients of OpenVac to have direct access to the stored
    /// geometry and perform a small local modification in-place might be
    /// critical for performance. Also, this allows to send a unique
    /// notification for a batch geometry edit of thousands of cells, which may
    /// also be critical for performance.
    ///
    /// Note how beginTopologyEdit() does not take any parameter, while
    /// beginGeometryEdit() does. The reason is that OpenVac knows which cells
    /// are affected by topological operators, but cannot know which cells are
    /// affected by a geometry edit, since geometry is user-defined.
    ///
    void beginGeometryEdit(const CellHandleSet & affected)
    {
        geometryEditAffected_ = affected;
    }

    /// Convenient overload of
    /// beginGeometryEdit(const std::vector<CellHandle> &),
    /// for when there is only one affected cell.
    ///
    void beginGeometryEdit(const CellHandle & affected)
    {
        geometryEditAffected_.clear();
        geometryEditAffected_.push_back(affected);
    }

    /// Clients must call this method when they are done editing the geometry.
    /// This will notify all observers that the geometry changed.
    ///
    void endGeometryEdit()
    {
        for (VacObserver * observer: observers_)
        {
            observer->geometryChanged(geometryEditAffected_);
        }
        geometryEditAffected_.clear();
    }

private:
    // Emits the topologyChanged() notification. This is called either by
    // Operator::apply() (when topology edits are not concatenated), or by
    // endTopologyEdit() (when topology edits are concatenated)
    //
    void emitTopologyChanged_()
    {
        for (VacObserver * observer: observers_)
        {
            observer->topologyChanged(
                        topologyEditCreated_,
                        topologyEditDestroyed_,
                        topologyEditAffected_);
        }

        areTopologyEditsConcatenated_ = false;
        topologyEditCreated_.clear();
        topologyEditDestroyed_.clear();
        topologyEditAffected_.clear();
    }

private:
    // Cell manager
    IdManager<SharedPtr<Cell>> cellManager_;

    // Geomety manager
    Geometry::GeometryManager geometryManager_;

    // Befriend Operator
    friend class Operator;

    // Observers
    std::unordered_set<VacObserver*> observers_;

    // Are topology edits concatenated or not? This variable is true if and
    // only if beginTopologyEdit() has been call by clients.
    //
    bool areTopologyEditsConcatenated_;

    // Cells whose topology is being edited. The value of these variables is
    // defined by the Operator class.
    //
    CellIdSet topologyEditCreated_;
    CellIdSet topologyEditDestroyed_;
    CellIdSet topologyEditAffected_;

    // Cells whose geometry is being edited
    CellHandleSet geometryEditAffected_;
};

} // end namespace OpenVac

#endif // OPENVAC_ VAC_H
