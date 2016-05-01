// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef CELLVISITOR_H
#define CELLVISITOR_H

namespace VectorAnimationComplex
{

class KeyVertex;
class KeyEdge;
class KeyFace;
class InbetweenVertex;
class InbetweenEdge;
class InbetweenFace;

class CellVisitor
{
public:
    CellVisitor();

    virtual void visit(KeyVertex * iVertex)=0;
    virtual void visit(KeyEdge * iEdge)=0;
    virtual void visit(KeyFace* iFace)=0;
    virtual void visit(InbetweenVertex * aVertex)=0;
    virtual void visit(InbetweenEdge * aEdge)=0;
    virtual void visit(InbetweenFace* aFace)=0;
};

}

#endif // CELLVISITOR_H
