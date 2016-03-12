// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_USINGGEOMETRY_H
#define OPENVAC_USINGGEOMETRY_H


/******************************* Private macro *******************************/

#define OPENVAC_USINGGEOMETRY_FRAME_(Prefix, MyGeometry) \
    using Prefix##Frame = typename MyGeometry::Frame;

#define OPENVAC_USINGGEOMETRY_CELL_GEOMETRY_(CellType, Prefix, MyGeometry) \
    using Prefix##CellType##Geometry = typename MyGeometry::CellType##Geometry;


/******************************* Public macro ********************************/

/// For each type name TypeName part of the \p Geometry concept, declares
/// the type alias \p PrefixTypeName for \p MyGeometry::TypeName
///
#define OPENVAC_USING_GEOMETRY(Prefix, MyGeometry) \
    OPENVAC_USINGGEOMETRY_FRAME_(Prefix, MyGeometry) \
    OPENVAC_FOREACH_FINAL_CELL_TYPE_ARGS( \
        OPENVAC_USINGGEOMETRY_CELL_GEOMETRY_, Prefix, MyGeometry)

#endif // OPENVAC_USINGGEOMETRY_H
