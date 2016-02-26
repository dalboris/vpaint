// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include <OpenVAC/Topology/KeyVertex.h>

namespace OpenVAC
{

KeyVertex::KeyVertex(VAC * vac, CellId id, const KeyVertexData & data) :
    Cell(vac, id),
    KeyCell(vac, id),
    VertexCell(vac, id),

    data_(data)
{
}

const KeyVertexData & KeyVertex::data() const
{
    return data_;
}

KeyVertexData & KeyVertex::data()
{
    return data_;
}

Frame KeyVertex::frame() const
{
    return data().frame;
}

}
