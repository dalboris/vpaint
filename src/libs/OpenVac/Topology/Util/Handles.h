// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_HANDLES
#define OPENVAC_HANDLES

#include <OpenVac/Core/ForeachCellType.h>
#include <OpenVac/Core/Memory.h>

namespace OpenVac
{

/************* Define and CellHandle classes *******************/

#define OPENVAC_HANDLES_FORWARD_DECLARE_CELL_(CellType) \
    class CellType; \
    /** Type alias for Handle<CellType> */ \
    using CellType##Handle = Handle<CellType>;

OPENVAC_FOREACH_CELL_TYPE(OPENVAC_HANDLES_FORWARD_DECLARE_CELL_)


/********************************* Handles ***********************************/

/// \class Handles OpenVac/Topology/Util/Handles.h
/// \brief Class that declares CellTypeRef as an alias for
/// Handle<CellType>, for each cell type.
///
/// Handles is used as the T template argument of the Data classes stored
/// inside the Cell classes.
///
class Handles
{
#define OPENVAC_HANDLES_USING_HANDLE_AS_REF_(CellType) \
    /** Type alias for CellType##Handle */ \
    using CellType##Ref = CellType##Handle;

public:
    OPENVAC_FOREACH_CELL_TYPE(OPENVAC_HANDLES_USING_HANDLE_AS_REF_)
};

} // end namespace OpenVac

#endif // OPENVAC_HANDLES
