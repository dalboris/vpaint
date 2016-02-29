// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_OPMAKEKEYVERTEX_H
#define OPENVAC_OPMAKEKEYVERTEX_H

#include <OpenVAC/Operators/Operator.h>

namespace OpenVAC
{

template <class Geometry>
class OpMakeKeyVertex: public Operator<Geometry>
{
public:
    OPENVAC_OPERATOR(OpMakeKeyVertex)

    // Constructor
    OpMakeKeyVertex(VAC * vac, Frame frame) :
        Operator(vac),
        frame_(frame) {}

    // Post-computation info. Aborts if not computed.
    KeyVertexId keyVertexId() const { assert(isComputed()); return keyVertexId_; }

    // Post-application info. Aborts if not applied.
    KeyVertexHandle keyVertex() const { assert(isApplied()); return vac()->cell(keyVertexId()); }

private:
    Frame frame_;
    bool isValid_() { return true; }

    KeyVertexId keyVertexId_;
    void compute_()
    {
        auto keyVertex = newKeyVertex(&keyVertexId_);
        keyVertex->frame = frame_;
    }
};

namespace Operators
{
template <class Geometry>
KeyVertexHandle<Geometry> makeKeyVertex(VAC<Geometry> * vac, Frame frame)
{
    return OpMakeKeyVertex<Geometry>(vac, frame).apply().keyVertex();
}
template <class Geometry>
OpMakeKeyVertex<Geometry> MakeKeyVertex(VAC<Geometry> * vac, Frame frame)
{
    return OpMakeKeyVertex<Geometry>(vac, frame);
}
}

}

#endif
