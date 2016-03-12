// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

/// \file OpenVac/Core/CellType.h
/// \brief Defines the scoped enumeration CellType.

#ifndef OPENVAC_CELLTYPE_H
#define OPENVAC_CELLTYPE_H

namespace OpenVac
{

/// \enum CellType
/// \brief A scoped enumeration of cell types.
///
/// The *type* of a cell is defined by its temporal dimension and its spatial
/// dimension. The temporal dimension of a cell can be either 0 (a "key cell")
/// or 1 (an "inbetween cell"), and its spatial dimension can be either 0 (a
/// "vertex cell"), 1 (an "edge cell"), or 2 (a "face cell").
///
/// Therefore, each cell has one of the following 6 types:
///     * OpenVac::CellType::KeyVertex
///     * OpenVac::CellType::KeyEdge
///     * OpenVac::CellType::KeyFace
///     * OpenVac::CellType::InbetweenVertex
///     * OpenVac::CellType::InbetweenEdge
///     * OpenVac::CellType::InbetweenFace
///
/// To query the type of a cell or cell data, use Cell:type() or
/// CellData::type().
///
/// \code
/// CellHandle cell = vac->cell(id);
/// if (cell->type() == OpenVac::CellType::KeyEdge)
/// {
///     std::cout << "I'm a key edge!\n";
/// }
/// \endcode
///
/// However, in most cases, you would need to cast the cell to its derived type
/// just after the if condition, and in these cases it is safer and more
/// readable to use the following syntax (i.e., "test the cast", instead of
/// "test the type, then cast"):
///
/// \code
/// // BAD
/// if (cell->type() == OpenVac::CellType::KeyEdge)
/// {
///     KeyEdgeHandle keyEdge = cell
///     KeyVertexHandle startVertex = keyEdge->startVertex();
///     KeyVertexHandle endVertex = keyEdge->endVertex();
///     // ...
/// }
///
/// // GOOD
/// if (KeyEdgeHandle keyEdge = cell)
/// {
///     KeyVertexHandle startVertex = keyEdge->startVertex();
///     KeyVertexHandle endVertex = keyEdge->endVertex();
///     // ...
/// }
/// \endcode
///
/// To query the "partial types" of a cell (i.e., query its spatial dimension
/// or temporal dimension separately), use operator&.
///
/// \code
/// CellHandle cell = vac->cell(id);
/// if (cell->type() & OpenVac::CellType::KeyCell)
/// {
///     std::cout << "I'm a key cell!\n";
/// }
/// \endcode
///
/// However, here again if you need to perform a cast after the if condition,
/// it is preferred to use the following syntax:
///
/// \code
/// CellHandle cell = vac->cell(id);
/// if (KeyCell keyCell = cell)
/// {
///     Frame frame = keyCell->frame();
///     // ...
/// }
/// \endcode
///
enum class CellType : char
{
    Cell       = 0x00,

    KeyCell       = 0x01,
    InbetweenCell = 0x02,

    VertexCell    = 0x10,
    EdgeCell      = 0x20,
    FaceCell      = 0x40,

    KeyVertex = KeyCell | VertexCell, ///< Some doc
    KeyEdge   = KeyCell | EdgeCell,
    KeyFace   = KeyCell | FaceCell,

    InbetweenVertex = InbetweenCell | VertexCell,
    InbetweenEdge   = InbetweenCell | EdgeCell,
    InbetweenFace   = InbetweenCell | FaceCell
};

inline constexpr CellType operator&(CellType t1, CellType t2)
{
    return static_cast<CellType> (static_cast<char>(t1) & static_cast<char>(t2));
}

} // end namespace OpenVac

#endif // OPENVAC_CELLTYPE_H
