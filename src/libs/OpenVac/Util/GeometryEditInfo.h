// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_GEOMETRYEDITINFO_H
#define OPENVAC_GEOMETRYEDITINFO_H

#include "OpenVac/Core/CellIdTypeSet.h"

namespace OpenVac
{

/// \class GeometryEditInfo
/// \brief A class that stores info about geometry edits.
///
class GeometryEditInfo
{
public:
    /// Constructs an empty GeometryEditInfo.
    ///
    GeometryEditInfo() :
        affected_()
    {
    }

    /// Constructs a GeometryEditInfo with the given changes.
    ///
    GeometryEditInfo(const CellIdTypeSet & affected) :
        affected_(affected)
    {
    }

    /// Clears the GeometryEditInfo.
    ///
    void clear()
    {
        affected_.clear();
    }

    /// Returns the set of affected cells.
    ///
    const CellIdTypeSet & affected() const
    {
        return affected_;
    }

    /// Compose this topology changes with the given changes.
    ///
    void compose(const GeometryEditInfo & other)
    {
        for (CellIdType c: other.affected_)
        {
            affected_.insert(c);
        }
    }

private:
    CellIdTypeSet affected_;
};

} // end namespace OpenVac

#endif // OPENVAC_GEOMETRYEDITINFO_H
