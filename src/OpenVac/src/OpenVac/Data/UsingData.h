// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_USINGDATA_H
#define OPENVAC_USINGDATA_H

#include <OpenVac/Core/ForeachCellType.h>
#include <OpenVac/Data/CellData.h>
#include <OpenVac/Data/KeyVertexData.h>
#include <OpenVac/Data/KeyEdgeData.h>


/*********************************** Private macro ***************************/

#define OPENVAC_USINGDATA_CELL_DATA_(CellType, Prefix, MyT, MyGeometry) \
    using Prefix##CellType##Data = OpenVac::CellType##Data<MyT, MyGeometry>;


/*********************************** Public macro ****************************/

/// For each class \p FooData in <OpenVac/Data/*.h>, declares the type alias
/// \p PrefixFooData for \p FooData<MyT, MyGeometry>.
///
#define OPENVAC_USING_DATA(Prefix, MyT, MyGeometry) \
    OPENVAC_FOREACH_CELL_DATA_TYPE_ARGS( \
        OPENVAC_USINGDATA_CELL_DATA_, Prefix, MyT, MyGeometry)


#endif // OPENVAC_USINGDATA_H
