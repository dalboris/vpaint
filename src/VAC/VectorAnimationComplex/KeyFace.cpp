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

#include "../XmlStreamReader.h"
#include "../XmlStreamWriter.h"
#include <QTextStream>
#include <QtDebug>
#include <QMessageBox>
#include <array>
#include <vector>

#include "../SaveAndLoad.h"
#include "../Random.h"

#include "EdgeGeometry.h"

#include "KeyVertex.h"
#include "KeyEdge.h"
#include "KeyFace.h"
#include "../DevSettings.h"
#include "../Global.h"
#include "../OpenGL.h"

namespace VectorAnimationComplex
{

namespace
{

detail::PolygonData createPolygonData(const QList<Cycle> & cycles)
{
    detail::PolygonData vertices;
    for(int k=0; k<cycles.size(); ++k)      // for each cycle
    {
        vertices << std::vector< std::array<double, 3> >(); // create a contour data

        for(int i=0; i<cycles[k].size(); ++i) // for each edge in the cycle
        {
            QList<Eigen::Vector2d> & sampling = cycles[k][i].edge->geometry()->sampling();
            if(cycles[k][i].side)
            {
                int last = sampling.size()-1;
                if(i+1<cycles[k].size())
                {
                    // if there is a following edge, then don't insert last vertex
                    // otherwise it will be duplicated
                    last--;
                }
                for(int j=0; j<=last; ++j)
                {
                    std::array<double, 3> a = {sampling[j][0], sampling[j][1], 0};
                    vertices.back().emplace_back(a);
                }
            }
            else
            {
                int first = 0;
                if(i+1<cycles[k].size())
                {
                    // if there is a following edge, then don't insert last vertex
                    // otherwise it will be duplicated
                    first++;
                }
                for(int j=sampling.size()-1; j>=first; --j)
                {
                    std::array<double, 3> a = {sampling[j][0], sampling[j][1], 0};
                    vertices.back().emplace_back(a);
                }
            }
        }
    }

    return vertices;
}

void computeTrianglesFromCycles(const QList<Cycle> & cycles, Triangles & triangles)
{
    detail::PolygonData polygon = createPolygonData(cycles);
    detail::tesselatePolygon(polygon, triangles);
}

}

// ------------------------------- Key Face -------------------------------

KeyFace::KeyFace(VAC * vac, const Time & t) :
    Cell(vac),
    KeyCell(vac, t),
    FaceCell(vac)
{
    initColor_();
}

KeyFace::KeyFace(VAC * vac, const Cycle & cycle) :
    Cell(vac),
    KeyCell(vac, cycle.time()),
    FaceCell(vac)
{
    addCycle(cycle);
    initColor_();
}

KeyFace::KeyFace(VAC * vac, const QList<Cycle> & cycles) :
    Cell(vac),
    KeyCell(vac, cycles[0].time()),
    FaceCell(vac)
{
    addCycles(cycles);
    initColor_();
}

void KeyFace::initColor_()
{
    // Random color
    /*
        // using hsv to rgb
        double H = Random::random(60./60.,200./60.);
        double S = 0.2;
        double V = 1;

        double C = V*S;
        double X = C * ( 1 - std::abs(std::fmod(H,2)-1) );
        double R1, G1, B1;
        if(H<1) { R1=C; G1=X; B1=0;}
        else if(H<2) { R1=X; G1=C; B1=0;}
        else if(H<3) { R1=0; G1=C; B1=X;}
        else if(H<4) { R1=0; G1=X; B1=C;}
        else if(H<5) { R1=X; G1=0; B1=C;}
        else { R1=C; G1=0; B1=X;}
        double m = V - C;
        color_[0] = R1+m;
        color_[1] = G1+m;
        color_[2] = B1+m;
        color_[3] = 1;
    */

    // Color from GUI
    setColor(global()->faceColor());
}

void KeyFace::clearCycles_()
{
    foreach(Cell * cell, spatialBoundary())
        removeMeFromSpatialStarOf_(cell);

    cycles_.clear();
    processGeometryChanged_();
}

void KeyFace::setCycles(const QList<Cycle> & cycles)
{
    clearCycles_();
    addCycles(cycles);
}

void KeyFace::addCycles(const QList<Cycle> & cycles)
{
    for(int i=0; i<cycles.size(); ++i)
        addCycle(cycles[i]);
}

void KeyFace::addCycle(const Cycle & cycle)
{
    cycles_ << cycle;
    foreach(KeyCell * cell, cycle.cells())
        addMeToSpatialStarOf_(cell);
    processGeometryChanged_();
}

void KeyFace::triangulate_(Time time, Triangles & out) const
{
    out.clear();
    if (exists(time))
        computeTrianglesFromCycles(cycles_, out);
}

QList<QList<Eigen::Vector2d> > KeyFace::getSampling(Time /*time*/) const
{
    QList<QList<Eigen::Vector2d> > res;
    detail::PolygonData data = createPolygonData(cycles_);

    for(unsigned int k=0; k<data.size(); ++k) // for each cycle
    {
        res << QList<Eigen::Vector2d>();
        for(unsigned int i=0; i<data[k].size(); ++i) // for each edge in the cycle
        {
            res[k] << Eigen::Vector2d(data[k][i][0], data[k][i][1]);
        }
    }

    return res;
}

QString KeyFace::xmlType_() const
{
    return "face";
}

void KeyFace::write_(XmlStreamWriter & xml) const
{
    // Base classes
    Cell::write_(xml);
    KeyCell::write_(xml);
    FaceCell::write_(xml);

    // Cycles
    QString cyclesstring;
    for(int i=0; i<cycles_.size(); ++i)
    {
        if(i>0)
            cyclesstring += " ";
        cyclesstring += cycles_[i].toString();
    }
    xml.writeAttribute("cycles", cyclesstring);
}

KeyFace::KeyFace(VAC * vac, XmlStreamReader & xml) :
    Cell(vac, xml),
    KeyCell(vac, xml),
    FaceCell(vac, xml)
{
    // Cycles
    QString str;
    QStringRef d = xml.attributes().value("cycles");
    bool opened = false;
    for(int i=0; i<d.length(); ++i)
    {
        QChar c = d.at(i);
        if(c == '[')
            opened = true;
        if(opened)
            str += c;
        if(c==']')
        {
            cycles_ << Cycle();
            cycles_.last().fromString(str);
            opened = false;
            str.clear();
        }
    }
}

void KeyFace::save_(QTextStream & out)
{
    // Base classes
    Cell::save_(out);
    KeyCell::save_(out);
    FaceCell::save_(out);

    // Cycles
    out << Save::newField("Cycles") << cycles_;
}

KeyFace::KeyFace(VAC * vac, QTextStream & in) :
    Cell(vac, in),
    KeyCell(vac, in),
    FaceCell(vac, in)
{
    Field field;

    // Cycles
    in >> /*field >>*/ cycles_; // Reason of comment: see Cell::Cell(VAC * vac, QTextStream & in)
}

void KeyFace::read2ndPass()
{
    // Base classes
    Cell::read2ndPass();
    KeyCell::read2ndPass();
    FaceCell::read2ndPass();

    // Cycles
    for(int i=0; i<cycles_.size(); ++i)
        cycles_[i].convertTempIdsToPointers(vac());
}

KeyFace::~KeyFace()
{
}

CellSet KeyFace::spatialBoundary() const
{
    CellSet res;

    for(int i=0; i<cycles_.size(); ++i)
    {
        CellSet cells = cycles_[i].cells();
        res.unite(cells);
    }

    return res;
}

void KeyFace::updateBoundary_impl(KeyEdge * oldEdge, const KeyEdgeList & newEdges)
{
    for(int i=0; i<cycles_.size(); ++i)
        cycles_[i].replaceEdges(oldEdge, newEdges);
}

// Update boundary
void KeyFace::updateBoundary_impl(KeyVertex * oldVertex, KeyVertex * newVertex)
{
    for(int i=0; i<cycles_.size(); ++i)
        cycles_[i].replaceVertex(oldVertex, newVertex);
}

void KeyFace::updateBoundary_impl(const KeyHalfedge & oldHalfedge, const KeyHalfedge & newHalfedge)
{
    for(int i=0; i<cycles_.size(); ++i)
        cycles_[i].replaceHalfedge(oldHalfedge, newHalfedge);
}

KeyFace * KeyFace::clone()
{
    return new KeyFace(this);
}

void KeyFace::remapPointers(VAC * newVAC)
{
    Cell::remapPointers(newVAC);
    KeyCell::remapPointers(newVAC);
    FaceCell::remapPointers(newVAC);

    for(int i=0; i<cycles_.size(); ++i)
        cycles_[i].remapPointers(newVAC);
}

KeyFace::KeyFace(KeyFace * other) :
    Cell(other),
    KeyCell(other),
    FaceCell(other)
{
    cycles_ = other->cycles_;
}

bool KeyFace::check_() const
{
    // todo
    return true;
}

// --------------------------- Preview Key Face ---------------------------


PreviewKeyFace::PreviewKeyFace() :
    cycles_()
{
}

PreviewKeyFace::PreviewKeyFace(const Cycle & cycle) :
    cycles_()
{
    cycles_ << cycle;
    computeTriangles_();
}

PreviewKeyFace::PreviewKeyFace(const QList<Cycle> & cycles) :
    cycles_(cycles)
{
}

void PreviewKeyFace::clear()
{
    cycles_.clear();
    triangles_.clear();
}

PreviewKeyFace & PreviewKeyFace::operator<< (const Cycle & cycle)
{
    cycles_ << cycle;
    computeTriangles_();
    return *this;
}

void PreviewKeyFace::computeTriangles_()
{
    computeTrianglesFromCycles(cycles_,triangles_);
}

bool PreviewKeyFace::intersects(double x, double y) const
{
    return triangles_.intersects(Eigen::Vector2d(x,y));
}

void PreviewKeyFace::draw(ViewSettings & /*viewSettings*/)
{
    // Setting the color
    glColor4d(0.9,0.9,0.9,0.8);

    // Drawing
    triangles_.draw();
}

}
