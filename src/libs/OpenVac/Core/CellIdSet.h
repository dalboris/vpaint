// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

/// \file OpenVac/Core/CellIdSet.h
/// \brief For each cell type CellType, declares CellTypeIdSet as an alias for
///  sdt::unordered_set<CellTypeId>.

#ifndef OPENVAC_CELLIDSET_H
#define OPENVAC_CELLIDSET_H

#include <OpenVac/Core/ForeachCellType.h>
#include <OpenVac/Core/CellId.h>

#include <unordered_set>

namespace OpenVac
{

#define OPENVAC_USING_CELL_ID_SET_(CellType) \
    /** Convenient type alias for unsigned int, for readability */ \
    using CellType##IdSet = std::unordered_set<CellType##Id>;

OPENVAC_FOREACH_CELL_TYPE(OPENVAC_USING_CELL_ID_SET_)

} // end namespace OpenVac

#endif // OPENVAC_CELLIDSET_H
