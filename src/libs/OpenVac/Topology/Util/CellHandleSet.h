// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_CELLHANDLESET_H
#define OPENVAC_CELLHANDLESET_H

#include <OpenVac/Core/ForeachCellType.h>
#include <OpenVac/Topology/Util/CellHandle.h>

#include <unordered_set>

namespace OpenVac
{

/// \class HandleHash  OpenVac/Topology/Util/CellHandleSet.h
/// \brief A functor class template to compute a hash key from a Handle<T>
///
/// This class is used to define CellHandleSet as a std::unordered_set
/// with a custom hash function.
///
template <class T>
struct HandleHash
{
    /// Returns a hash of the underlying pointer of the handle
    ///
    size_t operator() (const Handle<T> & handle) const
    {
        return std::hash<T*>(handle.get());
    }
};

#define OPENVAC_CELLHANDLESET_DEFINE_CELLHANDLESET_(CellType) \
    class CellType; \
    /** Type alias for Handle<CellType> */ \
    using CellType##HandleSet = std::unordered_set<Handle<CellType>, HandleHash<CellType>>;

OPENVAC_FOREACH_CELL_TYPE(OPENVAC_CELLHANDLESET_DEFINE_CELLHANDLESET_)

}

#endif // OPENVAC_CELLHANDLESET_H
