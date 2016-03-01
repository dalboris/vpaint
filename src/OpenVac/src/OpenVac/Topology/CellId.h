// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_CELL_ID_H
#define OPENVAC_CELL_ID_H

#include <OpenVac/Core/ForeachCellType.h>

namespace OpenVac
{

#define OPENVAC_DECLARE_CELL_ID_(CellType) \
    typedef unsigned int CellType##Id;

OPENVAC_FOREACH_CELL_TYPE(OPENVAC_DECLARE_CELL_ID_)

}

#endif // OPENVAC_CELL_ID_H
