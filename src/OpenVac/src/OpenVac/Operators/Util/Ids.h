// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_IDS_H
#define OPENVAC_IDS_H

#include <OpenVac/Core/ForeachCellType.h>
#include <OpenVac/Core/CellId.h>

namespace OpenVac
{

/// \class Ids OpenVac/Operator/Util/Ids.h
/// \brief Class that declares CellTypeRef as an alias for CellTypeId, for
/// each cell type.
///
/// Ids is used as the T template argument of the Data classes stored inside
/// the Operator classes.
///
class Ids
{
#define OPENVAC_IDS_USING_ID_AS_REF_(CellType) \
    /** \typedef CellType##Ref OpenVac/Core/CellId.h */ \
    /** Some doc. */ \
    using CellType##Ref = CellType##Id;

public:
    OPENVAC_FOREACH_CELL_TYPE(OPENVAC_IDS_USING_ID_AS_REF_)
};

} // end namespace OpenVac

#endif // OPENVAC_IDS_H
