// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_TKEYVERTEXDATA_H
#define OPENVAC_TKEYVERTEXDATA_H

#include <OpenVAC/Topology/TCellData/TCellData.h>
#include <OpenVAC/Core/Frame.h>

namespace OpenVAC
{

/// \class TKeyVertexData Topology/TCellData/TKeyVertexData.h
/// \brief TKeyVertexData is a class template to store low-level key vertex topological data.
///
/// The TKeyVertexData<T> class is not meant to be used by client code. Read
/// the TCellData<T> documentation first.

template <class T>
class TKeyVertexData : public TCellData<T>
{
public:
    // Type
    CellType type() const { return CellType::KeyVertex; }

    // Type casting to this class
    TKeyVertexData<T> * toKeyVertexData() { return this; }
    const TKeyVertexData<T> * toKeyVertexData() const { return this; }
    static TKeyVertexData<T> * cast(TCellData<T> * c) { return c ? c->toKeyVertexData() : nullptr; }
    static const TKeyVertexData<T> * cast(const TCellData<T> * c) { return c ? c->toKeyVertexData() : nullptr; }

    // Data
    Frame frame;
};

}

#endif
