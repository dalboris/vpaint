// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "Topology/KeyEdge.h"

namespace OpenVAC
{

KeyEdge::KeyEdge(VAC * vac, CellId id, const KeyEdgeData & data) :
    Cell(vac, id),
    KeyCell(vac, id),
    EdgeCell(vac, id),

    data_(data)
{
}

const KeyEdgeData & KeyEdge::data() const
{
    return data_;
}

KeyEdgeData & KeyEdge::data()
{
    return data_;
}

Frame KeyEdge::frame() const
{
    return data().frame;
}

KeyVertexHandle KeyEdge::startVertex() const
{
    return data().startVertex;
}

KeyVertexHandle KeyEdge::endVertex() const
{
    return data().endVertex;
}

}
