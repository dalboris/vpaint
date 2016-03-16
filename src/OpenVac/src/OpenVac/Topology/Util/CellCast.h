// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_CELLCAST_H
#define OPENVAC_CELLCAST_H

#include <OpenVac/Core/ForeachCellType.h>

/// \file OpenVac/Topology/Util/CellCast.h
///
/// This file define macros that implements handle_cast. It is equivalent in
/// behaviour to dynamic_cast, but 10x faster (implemented using virtual
/// functions, which we can do because the subclasses of Cell are fixed and
/// known in advanced).
///
/// Example:
///    Cell * cell = new KeyVertex();
///    KeyCell * keyCell = KeyCell::handle_cast(cell);
///
/// Is equivalent to:
///    Cell * cell = new KeyVertex();
///    KeyCell * keyCell = dynamic_cast<KeyCell*>(cell);

#define OPENVAC_CELL_DEFINE_TO_CELL_TYPE_STATIC_(CellType) \
    static CellType * to##CellType(Cell * c) \
    { \
        return c ? c->to##CellType##_() : nullptr; \
    }

#define OPENVAC_CELL_DEFINE_TO_CELL_TYPE_MEMBER_(CellType) \
    virtual CellType * to##CellType##_() \
    { \
        return nullptr; \
    }

#define OPENVAC_CELL_DEFINE_CAST_BASE_ \
    /** \cond Don't show those up in Doxygen */ \
    protected: \
        OPENVAC_FOREACH_CELL_TYPE(OPENVAC_CELL_DEFINE_TO_CELL_TYPE_STATIC_) \
    private: \
        OPENVAC_FOREACH_DERIVED_CELL_TYPE(OPENVAC_CELL_DEFINE_TO_CELL_TYPE_MEMBER_) \
    /** \endcond */

#define OPENVAC_CELL_DEFINE_CAST_(CellType) \
    /** \cond Don't show those up in Doxygen */ \
    template<class T, class U> \
    friend Handle<T> handle_cast(const Handle<U> & r); \
    \
    template<class T, class U> \
    friend Handle<T> handle_cast(const SharedPtr<U> & r); \
    \
    CellType * to##CellType##_() \
    { \
        return this; \
    } \
    \
    static CellType * handle_cast(Cell * c) \
    { \
        return Cell::to##CellType(c); \
    } \
    /** \endcond */

#endif // OPENVAC_CELLCAST_H
