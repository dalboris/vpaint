// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_TKEYEDGEDATA_H
#define OPENVAC_TKEYEDGEDATA_H

#include <OpenVAC/Topology/TCellData/TCellData.h>
#include <OpenVAC/Core/Frame.h>

namespace OpenVAC
{

/// \class TKeyEdgeData Topology/TCellData/TKeyEdgeData.h
/// \brief TKeyEdgeData is a class template to store low-level key edge topological data.
///
/// The TKeyEdgeData<T> class is not meant to be used by client code. Read
/// the TCellData<T> documentation first.

template <class T>
class TKeyEdgeData : public TCellData<T>
{
public:
    // Type
    CellType type() const { return CellType::KeyEdge; }

    // Type casting to this class
    TKeyEdgeData<T> * toKeyEdgeData() { return this; }
    const TKeyEdgeData<T> * toKeyEdgeData() const { return this; }
    static TKeyEdgeData<T> * cast(TCellData<T> * c) { return c ? c->toKeyEdgeData() : nullptr; }
    static const TKeyEdgeData<T> * cast(const TCellData<T> * c) { return c ? c->toKeyEdgeData() : nullptr; }

    // Data
    Frame frame;
    typename T::KeyVertexRef startVertex;
    typename T::KeyVertexRef endVertex;
};

}

#endif
