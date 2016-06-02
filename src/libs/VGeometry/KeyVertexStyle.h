// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef VGEOMETRY_KEYVERTEXSTYLE_H
#define VGEOMETRY_KEYVERTEXSTYLE_H

namespace VGeometry
{

/// \class KeyVertexStyle
/// \brief A struct that stores the style of a given KeyVertex
///
/// Note: for now, only CapStyle::Round and JoinStyle::Round are supported.
///
struct KeyVertexStyle
{
    enum class CapStyle
    {
        Round,
        Butt,
        Square
    };

    enum class JoinStyle
    {
        Round,
        Bevel,
        Miter, // Same as SVG miter-clip
        Arcs
    };

    enum class ClipStyle
    {
        Round,
        Bevel
    };

    /// Style to apply when there is only one incident edge.
    ///
    CapStyle capStyle = CapStyle::Round;

    /// Style to apply when there are two or more incident edges
    ///
    JoinStyle joinStyle = JoinStyle::Round;

    /// When joinStyle is Miter or Arcs, this value is used
    /// to know when to clip it
    ///
    double clipLimit = 0.0;

    /// When joinStyle is Miter or Arcs, and the clip limit is
    /// exceeded, this value specified whether it should fallback
    /// to a bevel or a round join.
    ///
    ClipStyle clipStyle = ClipStyle::Round;
};

} // end namespace VGeometry

#endif // VGEOMETRY_KEYVERTEXSTYLE_H
