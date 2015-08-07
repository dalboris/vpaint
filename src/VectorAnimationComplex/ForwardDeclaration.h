// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef VAC_FORWARD_DECLARATION_H
#define VAC_FORWARD_DECLARATION_H

namespace VectorAnimationComplex
{

class VAC;

// -------- Cell Virtual Classes --------

// Base
class Cell;

// Temporal Behaviour
class KeyCell;
class InbetweenCell;

// Spatial Behaviour
class VertexCell;
class EdgeCell;
class FaceCell;

// ------ Cell Non-Virtual Classes ------

class KeyVertex;
class KeyEdge;
class KeyFace;

class InbetweenVertex;
class InbetweenEdge;
class InbetweenFace;

}

#endif
