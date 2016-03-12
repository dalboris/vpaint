// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_TKEYEDGEDATA_H
#define OPENVAC_TKEYEDGEDATA_H

#include <OpenVac/Data/CellData.h>

namespace OpenVac
{

/// \class KeyEdgeData OpenVac/Data/KeyEdgeData.h
/// \brief A class to store key edge raw data.
///
/// \sa CellData

template <class T, class Geometry>
class KeyEdgeData : public CellData<T, Geometry>
{
public:
    // Type
    CellType type() const { return CellType::KeyEdge; }

    // Visitor pattern
    void accept(CellDataVisitor<T, Geometry> & v) const { v.visit(*this); }
    void accept(CellDataMutator<T, Geometry> & m)       { m.visit(*this); }

    // Topological data
    typename T::KeyVertexRef startVertex;
    typename T::KeyVertexRef endVertex;

    // Geometric data
    typename Geometry::Frame frame;
    typename Geometry::KeyEdgeGeometry geometry;
};

} // end namespace OpenVac

#endif
