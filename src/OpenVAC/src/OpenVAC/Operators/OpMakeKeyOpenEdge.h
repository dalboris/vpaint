// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_OPMAKEKEYEDGE_H
#define OPENVAC_OPMAKEKEYEDGE_H

#include <OpenVAC/Operators/Operator.h>

namespace OpenVAC
{

template <class Geometry>
class OpMakeKeyOpenEdge: public Operator<Geometry>
{
public:
    OPENVAC_OPERATOR(OpMakeKeyOpenEdge)

    OpMakeKeyOpenEdge(KeyVertexHandle startVertex, KeyVertexHandle endVertex) :
        Operator(startVertex ? startVertex->vac() : nullptr),
        startVertex_(startVertex),
        endVertex_(endVertex) {}

    // Post-computation info. Aborts if not computed.
    KeyEdgeId keyEdgeId() const { assert(isComputed()); return keyEdgeId_; }

    // Post-application info. Aborts if not applied.
    KeyEdgeHandle keyEdge() const { assert(isApplied()); return vac()->cell(keyEdgeId()); }

private:
    KeyVertexHandle startVertex_, endVertex_;
    bool isValid_()
    {
        return     startVertex_
                && endVertex_
                && startVertex_->frame() == endVertex_->frame();
    }

    KeyEdgeId keyEdgeId_;
    void compute_()
    {
        auto keyEdge = newKeyEdge(&keyEdgeId_);
        keyEdge->frame       = startVertex_->frame();
        keyEdge->startVertex = startVertex_->id();
        keyEdge->endVertex   = endVertex_->id();
    }
};

namespace Operators
{
template <class Geometry>
OpMakeKeyOpenEdge<Geometry> MakeKeyOpenEdge(KeyVertexHandle<Geometry> startVertex, KeyVertexHandle<Geometry> endVertex)
{
    return OpMakeKeyOpenEdge<Geometry>(startVertex, endVertex);
}
template <class Geometry>
KeyEdgeHandle<Geometry> makeKeyOpenEdge(KeyVertexHandle<Geometry> startVertex, KeyVertexHandle<Geometry> endVertex)
{
    return MakeKeyOpenEdge(startVertex, endVertex).apply().keyEdge();
}
}

}

#endif
