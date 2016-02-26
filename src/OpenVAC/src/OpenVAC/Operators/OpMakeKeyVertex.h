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

class OpMakeKeyVertex: public Operator
{
public:
    OpMakeKeyVertex(VAC * vac, Frame frame);

private:
    bool isValid_();
    void compute_();

    Frame frame_;
};

}

#endif // OPENVAC_OPMAKEKEYVERTEX_H
