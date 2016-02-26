// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include <OpenVAC/Operators/OpMakeKeyVertex.h>
#include <OpenVAC/Topology/KeyVertex.h>
#include <OpenVAC/VAC.h>

#include <cassert>

namespace OpenVAC
{

OpMakeKeyVertex::OpMakeKeyVertex(VAC * vac, Frame frame) :
    Operator(vac),
    frame_(frame)
{
}

bool OpMakeKeyVertex::isValid_()
{
    return true;
}

KeyVertexId OpMakeKeyVertex::keyVertexId() const
{
    assert(isComputed());
    return keyVertexId_;
}

KeyVertexHandle OpMakeKeyVertex::keyVertex() const
{
    assert(isApplied());
    return vac()->cell(keyVertexId());
}

void OpMakeKeyVertex::compute_()
{
    auto keyVertex = newKeyVertex(&keyVertexId_);
    keyVertex->frame = frame_;
}

}
