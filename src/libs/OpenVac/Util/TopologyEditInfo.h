// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_TOPOLOGYEDITINFO_H
#define OPENVAC_TOPOLOGYEDITINFO_H

#include "OpenVac/Core/CellIdTypeSet.h"

namespace OpenVac
{

/// \class TopologyEditInfo
/// \brief A class that stores info about topology edits.
///
class TopologyEditInfo
{
public:
    /// Constructs an empty TopologyEditInfo.
    ///
    TopologyEditInfo() :
        created_(),
        destroyed_(),
        affected_()
    {
    }

    /// Constructs a TopologyEditInfo with the given changes.
    ///
    TopologyEditInfo(
            const CellIdTypeSet & created,
            const CellIdTypeSet & destroyed,
            const CellIdTypeSet & affected) :
        created_(created),
        destroyed_(destroyed),
        affected_(affected)
    {
    }

    /// Clears the TopologyEditInfo.
    ///
    void clear()
    {
        created_.clear();
        destroyed_.clear();
        affected_.clear();
    }

    /// Returns the set of created cells.
    ///
    const CellIdTypeSet & created() const
    {
        return created_;
    }

    /// Returns the set of destroyed cells.
    ///
    const CellIdTypeSet & destroyed() const
    {
        return destroyed_;
    }

    /// Returns the set of affected cells. Affected cells are
    /// cells which are neither created or affected, but whose
    /// boundary changed. For instance, when merging two vertices,
    /// the two vertices are destroyed, a new vertex is created,
    /// and the incident edges are affected (their boundary now points
    /// to the new vertex).
    ///
    const CellIdTypeSet & affected() const
    {
        return affected_;
    }

    /// Compose this topology edits with the given edits.
    ///
    void compose(const TopologyEditInfo & other)
    {
        for (CellIdType c: other.destroyed_)
        {
            created_.erase(c);
            affected_.erase(c);
            destroyed_.insert(c);
        }
        for (CellIdType c: other.created_)
        {
            created_.insert(c);
        }
        for (CellIdType c: other.affected_)
        {
            affected_.insert(c);
        }
    }

private:
    CellIdTypeSet created_;
    CellIdTypeSet destroyed_;
    CellIdTypeSet affected_;
};

} // end namespace OpenVac

#endif // OPENVAC_TOPOLOGYEDITINFO_H
