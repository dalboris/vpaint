// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef VAC_INBETWEENHALFEDGE_H
#define VAC_INBETWEENHALFEDGE_H

#include "HalfedgeBase.h"
#include "InbetweenEdge.h"

namespace VectorAnimationComplex
{

class InbetweenHalfedge: public HalfedgeBase<InbetweenEdge>
{
};

}

#endif // VAC_INBETWEENHALFEDGE_H
