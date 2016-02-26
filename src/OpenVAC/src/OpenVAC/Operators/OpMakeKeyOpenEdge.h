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

class OpMakeKeyOpenEdge: public Operator
{
public:
    OpMakeKeyOpenEdge(KeyVertexHandle startVertex, KeyVertexHandle endVertex);

    // Overrides compute() and apply() to return the derived type
    OPENVAC_OPERATOR_OVERRIDE_COMPUTE_AND_APPLY(OpMakeKeyOpenEdge)

    // Post-computation info. Aborts if not computed.
    KeyEdgeId keyEdgeId() const;

    // Post-application info. Aborts if not applied.
    KeyEdgeHandle keyEdge() const;

private:
    bool isValid_();
    void compute_();

    // In
    KeyVertexHandle startVertex_, endVertex_;

    // Out
    KeyEdgeId keyEdgeId_;
};

}

#endif
