// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

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
