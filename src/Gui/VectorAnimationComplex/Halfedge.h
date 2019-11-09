// Copyright (C) 2012-2019 The VPaint Developers
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
