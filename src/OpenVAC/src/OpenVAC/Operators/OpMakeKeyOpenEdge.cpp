// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include <OpenVAC/Operators/OpMakeKeyOpenEdge.h>
#include <OpenVAC/Topology/KeyVertex.h>
#include <OpenVAC/Topology/KeyEdge.h>
#include <OpenVAC/VAC.h>

#include <cassert>

namespace OpenVAC
{

OpMakeKeyOpenEdge::OpMakeKeyOpenEdge(KeyVertexHandle startVertex, KeyVertexHandle endVertex) :
    Operator(startVertex ? startVertex->vac() : nullptr),
    startVertex_(startVertex),
    endVertex_(endVertex)
{
}

bool OpMakeKeyOpenEdge::isValid_()
{
    return     startVertex_
            && endVertex_
            && startVertex_->frame() == endVertex_->frame();
}

KeyEdgeId OpMakeKeyOpenEdge::keyEdgeId() const
{
    assert(isComputed());
    return keyEdgeId_;
}

KeyEdgeHandle OpMakeKeyOpenEdge::keyEdge() const
{
    assert(isApplied());
    return vac()->cell(keyEdgeId());
}

void OpMakeKeyOpenEdge::compute_()
{
    auto keyEdge = newKeyEdge(&keyEdgeId_);
    keyEdge->frame       = startVertex_->frame();
    keyEdge->startVertex = startVertex_->id();
    keyEdge->endVertex   = endVertex_->id();
}

}
