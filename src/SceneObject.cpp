// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "SceneObject.h"
#include "VectorAnimationComplex/VAC.h"

#include <QTextStream>
#include "SaveAndLoad.h"

void SceneObject::save(QTextStream & out)
{
    // SceneObject Type
    out << Save::newField("SceneObject") << stringType();

    // Save Derived members
    save_(out);
}

void SceneObject::exportSVG(Time t, QTextStream & out)
{
    // Save Derived members
    exportSVG_(t, out);
}

void SceneObject::save_(QTextStream & /*out*/)
{
}

void SceneObject::exportSVG_(Time /*t*/, QTextStream & /*out*/)
{
}

SceneObject * SceneObject::read(QTextStream & in)
{
    QString field, type;
    field = Read::field(in);
    in >> type;

    if(type == "VectorAnimationComplex" || type == "VectorGraphicsComplex" )
    {
        return new VectorAnimationComplex::VAC(in);
    }

    else return 0;
}



