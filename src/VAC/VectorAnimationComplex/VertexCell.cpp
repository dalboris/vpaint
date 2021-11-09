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

#include "VertexCell.h"
#include "EdgeCell.h"
#include "FaceCell.h"
#include "KeyVertex.h"
#include "KeyEdge.h"
#include "../OpenGL.h"
#include "../DevSettings.h"
#include <QTextStream>
#include <QStringList>
#include "../SaveAndLoad.h"
#include "../Global.h"
#include "CellList.h"

#include <limits>

#include <QtDebug>

namespace VectorAnimationComplex
{

VertexCell::VertexCell(VAC * vac) :
    Cell(vac)
{
    colorSelected_[0] = 0.7;
    colorSelected_[1] = 0;
    colorSelected_[2] = 0;
    colorSelected_[3] = 1;
}

VertexCell::~VertexCell()
{
}

VertexCell::VertexCell(VAC * vac, QTextStream & in) :
    Cell(vac, in)
{
    colorSelected_[0] = 0.7;
    colorSelected_[1] = 0;
    colorSelected_[2] = 0;
    colorSelected_[3] = 1;
}

VertexCell::VertexCell(VAC * vac, XmlStreamReader & xml) :
    Cell(vac, xml)
{
    colorSelected_[0] = 0.7;
    colorSelected_[1] = 0;
    colorSelected_[2] = 0;
    colorSelected_[3] = 1;
}

bool VertexCell::isPickableCustom(Time /*time*/) const
{
    const bool verticesArePickable = true;
    if(
       verticesArePickable &&
       ( global()->toolMode() == Global::SELECT ||
         global()->toolMode() == Global::SCULPT )
       )
    {
        return true;
    }
    else
        return false;
}

void VertexCell::drawPickCustom(Time time, ViewSettings & /*viewSettings*/)
{
    if(!exists(time))
        return;

    int n = 50;
    Eigen::Vector2d p = pos(time);
    glBegin(GL_POLYGON);
    {
        double r = 0.5 * size(time);
        for(int i=0; i<n; ++i)
        {
            double theta = 2 * (double) i * 3.14159 / (double) n ;
            glVertex2d(p.x() + r*std::cos(theta),p.y()+ r*std::sin(theta));
            }
    }
    glEnd();
}

void VertexCell::drawRaw(Time time, ViewSettings & viewSettings)
{
    if(isHighlighted() || isSelected())
    {
        Cell::drawRaw(time, viewSettings);
    }
}

void VertexCell::drawRawTopology(Time time, ViewSettings & viewSettings)
{
    bool screenRelative = viewSettings.screenRelative();
    if(screenRelative)
    {
        int n = 50;
        Eigen::Vector2d p = pos(time);
        glBegin(GL_POLYGON);
        {
            double r = 0.5 * viewSettings.vertexTopologySize() / viewSettings.zoom();
            //if(r == 0) r = 3;
            //else if (r<1) r = 1;
            for(int i=0; i<n; ++i)
            {
                double theta = 2 * (double) i * 3.14159 / (double) n ;
                glVertex2d(p.x() + r*std::cos(theta),p.y()+ r*std::sin(theta));
            }
        }
        glEnd();
    }
    else
    {
        int n = 50;
        Eigen::Vector2d p = pos(time);
        glBegin(GL_POLYGON);
        {
            double r = 0.5 * viewSettings.vertexTopologySize();
            if(r == 0) r = 3;
            else if (r<1) r = 1;
            for(int i=0; i<n; ++i)
            {
                double theta = 2 * (double) i * 3.14159 / (double) n ;
                glVertex2d(p.x() + r*std::cos(theta),p.y()+ r*std::sin(theta));
            }
        }
        glEnd();
    }
}

double VertexCell::size(Time time) const
{
    double defaultSize = 0;

    // get outgoing halfedges
    QList<Halfedge> incidentEdgesT = incidentEdges(time);

    // valence == 0
    if(incidentEdgesT.isEmpty())
        return defaultSize;

    // valence > 0
    double res = 0; //std::numeric_limits<double>::max();
    for(Halfedge h: incidentEdgesT)
    {
        EdgeSample sample = h.startSample(time);
        if(sample.width() > res)
            res = sample.width();
    }
    return res;
}

void VertexCell::read2ndPass()
{
}

void VertexCell::save_(QTextStream & /*out*/)
{
}

void VertexCell::remapPointers(VAC * /*newVAC*/)
{
}

void VertexCell::write_(XmlStreamWriter & /*xml*/) const
{

}

VertexCell::VertexCell(VertexCell * other) :
    Cell(other)
{
    colorSelected_[0] = 0.7;
    colorSelected_[1] = 0;
    colorSelected_[2] = 0;
    colorSelected_[3] = 1;
}

CellSet VertexCell::spatialBoundary() const
{
    return CellSet();
}

CellSet VertexCell::spatialBoundary(Time /*t*/) const
{
    return CellSet();
}

QList<Halfedge> VertexCell::incidentEdges(Time t) const
{
    // Get key edges and inbetween edges in spatial star
    CellSet spatialStarT = spatialStar(t);
    KeyEdgeSet keyEdges = spatialStarT;
    InbetweenEdgeSet inbetweenEdges = spatialStarT;

    // Orient them so that "start(h) = this"
    // Note: Possibly add them twice if start = end = this
    QList<Halfedge> res;
    for(KeyEdge * keyEdge: keyEdges)
    {
        if(keyEdge->startVertex()->toVertexCell() == this)
            res << Halfedge(keyEdge, true);
        if(keyEdge->endVertex()->toVertexCell() == this)
            res << Halfedge(keyEdge, false);
    }
    for(InbetweenEdge * inbetweenEdge: inbetweenEdges)
    {
        if(inbetweenEdge->startVertex(t)->toVertexCell()  == this)
            res << Halfedge(inbetweenEdge, true);
        if(inbetweenEdge->endVertex(t)->toVertexCell() == this)
            res << Halfedge(inbetweenEdge, false);
    }

    return res;
}

bool VertexCell::checkVertex_() const
{
    // todo
    return true;
}

namespace
{

Eigen::Vector2d circle_(const Eigen::Vector2d & center, double r, double theta)
{
    return Eigen::Vector2d(center[0] + r * std::cos(theta),
                           center[1] + r * std::sin(theta));
}

}

void VertexCell::triangulate_(Time time, Triangles & out) const
{
    out.clear();
    if (exists(time))
    {
        const Eigen::Vector2d center = pos(time);
        const double r = 0.5 * size(time);

        const int n = 50;
        const double dTheta = 2 * 3.14159 / (double) n ;

        double theta = 0;
        Eigen::Vector2d a = circle_(center, r, theta);
        Eigen::Vector2d b;
        for(int i=0; i<n; ++i)
        {
            theta += dTheta;
            b = circle_(center, r, theta);
            out << Triangle(center, a, b);
            a = b;
        }
    }
}

void VertexCell::computeOutlineBoundingBox_(Time t, BoundingBox & out) const
{
    if (exists(t))
    {
        const Eigen::Vector2d center = pos(t);
        out = BoundingBox(center[0], center[1]);
    }
    else
    {
        out = BoundingBox();
    }
}

}
