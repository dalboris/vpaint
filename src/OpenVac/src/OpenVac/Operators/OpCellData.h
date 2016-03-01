// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_OPCELLDATA_H
#define OPENVAC_OPCELLDATA_H

#include <OpenVac/Core/Memory.h>
#include <OpenVac/Topology/CellId.h>

#define OPENVAC_USING_CELLID_AS_CELLREF_(CellType) \
    typedef CellId CellType##Ref;

#define OPENVAC_DECLARE_OP_CELL_DATA_(CellType) \
    template <class Geometry> \
    using Op##CellType##Data = T##CellType##Data<IdsAsRefs, Geometry>;

#define OPENVAC_DECLARE_OP_CELL_DATA_SHARED_PTR_(CellType) \
    template <class Geometry> \
    using Op##CellType##DataSharedPtr = SharedPtr<Op##CellType##Data<Geometry>>;

#define OPENVAC_DECLARE_OP_CELL_DATA_PTR_(CellType) \
    template <class Geometry> using Op##CellType##DataPtr = WeakPtr<Op##CellType##Data<Geometry>>;

namespace OpenVac
{

class IdsAsRefs
{
public:
    OPENVAC_FOREACH_CELL_TYPE(OPENVAC_USING_CELLID_AS_CELLREF_)
};

OPENVAC_FOREACH_CELL_DATA_TYPE(OPENVAC_DECLARE_OP_CELL_DATA_)
OPENVAC_FOREACH_CELL_DATA_TYPE(OPENVAC_DECLARE_OP_CELL_DATA_SHARED_PTR_)
OPENVAC_FOREACH_CELL_DATA_TYPE(OPENVAC_DECLARE_OP_CELL_DATA_PTR_)

}

#endif
