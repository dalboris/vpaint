// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_CELLIDTYPESET_H
#define OPENVAC_CELLIDTYPESET_H

#include "OpenVac/Core/CellIdType.h"

#include <unordered_set>

namespace OpenVac
{

struct CellIdTypeHash
{
    /// Returns a hash of the id-type pair.
    ///
    std::size_t operator() (const CellIdType & idtype) const
    {
        std::size_t h1 = std::hash<CellId>()(idtype.id);
        std::size_t h2 = std::hash<char>()(static_cast<char>(idtype.type));
        return h1 ^ (h2 << 1); // or use boost::hash_combine
    }
};

using CellIdTypeSet = std::unordered_set<CellIdType, CellIdTypeHash>;

} // end namespace OpenVac

#endif // OPENVAC_CELLIDTYPESET_H
