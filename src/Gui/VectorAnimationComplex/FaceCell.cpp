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

#include "FaceCell.h"
#include <QTextStream>
#include "../SaveAndLoad.h"
#include "../DevSettings.h"
#include "../Global.h"


namespace VectorAnimationComplex
{

FaceCell::FaceCell(VAC * vac) :
    Cell(vac)
{
    // highlighted/selected color
    colorSelected_[0] = 1;
    colorSelected_[1] = 0.5;
    colorSelected_[2] = 0.5;
    colorSelected_[3] = 1;
}

FaceCell::FaceCell(VAC * vac, XmlStreamReader & xml) :
    Cell(vac, xml)
{
    // highlighted/selected color
    colorSelected_[0] = 1;
    colorSelected_[1] = 0.5;
    colorSelected_[2] = 0.5;
    colorSelected_[3] = 1;
}

FaceCell::FaceCell(VAC * vac, QTextStream & in) :
    Cell(vac, in)
{
    // highlighted/selected color
    colorSelected_[0] = 1;
    colorSelected_[1] = 0.5;
    colorSelected_[2] = 0.5;
    colorSelected_[3] = 1;
}

void FaceCell::drawRawTopology(Time time, ViewSettings & viewSettings)
{
    if(viewSettings.drawTopologyFaces())
        triangles(time).draw();
}

bool FaceCell::isPickableCustom(Time /*time*/) const
{
    const bool areFacesPickable = true;
    if(areFacesPickable && global()->toolMode() == Global::SELECT)
        return true;
    else if(global()->toolMode() == Global::PAINT)
        return true;
    else if(global()->toolMode() == Global::SKETCH) // to detect which faces are hovered in planar map mode
        return true;
    else
        return false;
}

void FaceCell::computeOutlineBoundingBox_(Time t, BoundingBox & out) const
{
    out = boundingBox(t);
}

void FaceCell::exportSVG(Time t, QTextStream & out)
{
    // Get polygon data
    QList< QList<Eigen::Vector2d> > samples = getSampling(t);

    // Write file
    out << "<path d=\"";
    for(int k=0; k<samples.size(); ++k) // for each cycle
    {
        if(samples[k].size() < 2)
            continue;

        Eigen::Vector2d v0 = samples[k][0];
        out << "M " << v0[0] << "," << v0[1] << " ";

        for(int i=0; i<samples[k].size(); ++i) // for each vertex in cycle
        {
            Eigen::Vector2d v = samples[k][i];

            // safeguard against NaN and other oddities
            const double MAX_VALUE = 10000;
            const double MIN_VALUE = -10000;
            if( v[0] > MIN_VALUE &&
                    v[0] < MAX_VALUE &&
                    v[1] > MIN_VALUE &&
                    v[1] < MAX_VALUE )
            {
                out << "L " << v[0] << "," << v[1] << " ";
            }
        }
        out << "Z ";
    }
    out << "\" style=\""
        << "fill:rgb("
        << (int) (color_[0]*255) << ","
        << (int) (color_[1]*255) << ","
        << (int) (color_[2]*255) << ");"
        << "fill-opacity:" << color_[3] << ";"
        << "fill-rule:evenodd;stroke:none\" />\n";

}

void FaceCell::read2ndPass()
{
}

void FaceCell::save_(QTextStream & /*out*/)
{
}

FaceCell::~FaceCell()
{
}

FaceCell::FaceCell(FaceCell * other) :
    Cell(other)
{
}

void FaceCell::remapPointers(VAC * /*newVAC*/)
{
}

bool FaceCell::checkFace_() const
{
    // todo
    return true;
}

}
