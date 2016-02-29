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

template <class T, class Geometry>
class TKeyVertexData : public TCellData<T, Geometry>, public Geometry::KeyVertex
{
public:
    // Type
    CellType type() const { return CellType::KeyVertex; }

    // Type casting
    OPENVAC_DEFINE_CELLDATA_CAST(KeyVertex)

    // Topological data
    Frame frame;

    // Geometric data
    typedef typename Geometry::KeyVertex KeyVertexGeometry;
    KeyVertexGeometry & geometry() { return *this; }
    const KeyVertexGeometry & geometry() const { return *this; }
};

}

#endif
