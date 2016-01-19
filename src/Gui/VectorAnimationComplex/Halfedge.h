// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

/* -------------------- Halfedge.h --------------------
 *
 * In a nutshell, this file defines the following three classes:
 *
 *    struct Halfedge          { EdgeCell *      edge;  bool side; };
 *    struct KeyHalfedge       { KeyEdge *       edge;  bool side; };
 *    struct InbetweenHalfedge { InbetweenEdge * edge;  bool side; };
 *
 * Note that KeyHalfedge and InbetweenHalfedge do NOT inherit from Halfedge.
 * In practice, to factorize code, we define a templated class:
 *
 *    template <class TEdge> struct HalfedgeBase { TEdge * edge;  bool side; [+ methods common to all 3 classes] };
 *
 * Which is used to define our three classes:
 *
 * typedef HalfedgeBase<EdgeCell> Halfedge;
 * class KeyHalfedge:       public HalfedgeBase<KeyEdge>       { [methods specific to KeyHalfedge] };
 * class InbetweenHalfedge: public HalfedgeBase<InbetweenEdge> { [methods specific to InbetweenHalfedge] };
 *
 */

#ifndef VAC_HALFEDGE_H
#define VAC_HALFEDGE_H

#include "HalfedgeBase.h"
#include "KeyHalfedge.h"
#include "InbetweenHalfedge.h"

#endif
