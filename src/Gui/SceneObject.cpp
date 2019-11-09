// Copyright (C) 2012-2019 The VPaint Developers.
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



