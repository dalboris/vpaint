// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef SCENEOBJECTVISITOR_H
#define SCENEOBJECTVISITOR_H

namespace VectorAnimationComplex { class VAC; }

class SceneObjectVisitor
{
public:
    SceneObjectVisitor();

    virtual void visit(VectorAnimationComplex::VAC * vac)=0;
};

#endif // SCENEOBJECTVISITOR_H
