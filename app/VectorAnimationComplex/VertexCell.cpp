// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

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

void VertexCell::drawRaw(Time time, ViewSettings & /*viewSettings*/)
{
    if(!exists(time))
        return;

    if(isHighlighted() || isSelected())
    {
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

    //drawEdgeJunction(time, viewSettings);
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
    foreach(Halfedge h, incidentEdgesT)
    {
        EdgeSample sample = h.startSample(time);
        if(sample.width() > res)
            res = sample.width();
    }
    return res;
}

void VertexCell::drawEdgeJunction(Time /*time*/, ViewSettings & /*viewSettings*/)
{
    // NOTE: Because all the below had issues, Now I only support round joins.
    //       the joins are directly drawn as caps of each edge.
    //       Makes it more consistent and predictible. And user can choose "color"
    //       of the join by simply putting the desired edge on top

    /* METHOD 1: Rounded junction
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
    */


    /* METHOD 2: Miter junction (Note: old code from VGC, has to be ported to VAC)
    if(!exists(time))
        return;

    // in case we need to draw in black and white
    bool drawBlackAndWhite = !Settings::getBool("colored edges");

    // Get outgoing halfedges
    struct AngleHalfedge
    {
        Eigen::Vector2d u;
        double angle;
        double r;

        EIGEN_MAKE_ALIGNED_OPERATOR_NEW

        AngleHalfedge(EdgeCell * e, bool side)
        {
            u = h.leftDer();
            angle = std::atan2(u[1],u[0]);

            // dirty for now: must create abstract methods for this
            // get half-width of edge at this endpoint
            LinearSpline * ls = static_cast<LinearSpline *>(h.edge->geometry());
            int n = ls->size();
            r = 1;
            if(n > 0)
            {
                if(h.side)
                    r = 0.5 * (*ls)[0].width();
                else
                    r = 0.5 * (*ls)[n-1].width();
            }
        }

        AngleHalfedge(const InbetweenHalfEdge & h_) : h(h_)
        {
            u = h.leftDer();
            angle = std::atan2(u[1],u[0]);

            // dirty for now: must create abstract methods for this
            // get half-width of edge at this endpoint
            LinearSpline * ls = static_cast<LinearSpline *>(h.edge->geometry());
            int n = ls->size();
            r = 1;
            if(n > 0)
            {
                if(h.side)
                    r = 0.5 * (*ls)[0].width();
                else
                    r = 0.5 * (*ls)[n-1].width();
            }
        }

        bool operator< (const AngleHalfedge & other) const
        {
            return angle < other.angle;
        }
    };
    QList<AngleHalfedge> angleHalfedges;



    CellSet incidentEdges = spatialStar();
    QList<KeyHalfedge> outgoingHalfedges;
    foreach(KeyEdge * iedge, incidentEdges)
    {
        if(iedge->startVertex() == this)
            outgoingHalfedges << KeyHalfedge(iedge, true);
        if(iedge->endVertex() == this)
            outgoingHalfedges << KeyHalfedge(iedge, false);
    }

    foreach(KeyHalfedge h, outgoingHalfedges)
    {
        // do not include 0-width edges
        AngleHalfedge ah = AngleHalfedge(h);
        if(ah.r > 0)
            angleHalfedges << AngleHalfedge(h);
    }
    qSort(angleHalfedges);

    ///// print for debugging /////
    //qDebug() << "Angles around vertex" << id();
    //foreach(AngleHalfedge ah, angleHalfedges)
    //    qDebug() << "edge:" << ah.h.edge->id() << "; side:" <<  ah.h.side << "; angle:" << ah.angle;
    //qDebug() << "\n";
    ///////////////////////////////

    // for each adjacent pair, draw the improved junction
    double pi = 3.14159265;
    int n = angleHalfedges.size();
    if(n > 1)
    {
        for(int i=0; i<n; i++)
        {
            // get pair
            int iPlus1 = i+1;
            if(iPlus1==n)
                iPlus1 = 0;
            AngleHalfedge ah1 = angleHalfedges[i];
            AngleHalfedge ah2 = angleHalfedges[iPlus1];

            // get angles
            double angle1 = ah1.angle;
            double angle2 = ah2.angle;
            if(i==n-1)
                angle2 += 2 * pi;
            double dAngle = angle2 - angle1;

            // only draw nice junction when dAngle more than flat
            double threshold = pi/8;
            bool doThreshold = Settings::getBool("threshold edge junctions");
            bool flatStyle = Settings::getBool("flat edge junctions");
            if(dAngle > pi)
            {
                ///// print for debugging /////
                //qDebug() << "Drawing junction for" << "edge:" << ah1.h.edge->id() << "; side:" <<  ah1.h.side << "; angle:" << ah1.angle;
                //qDebug() << "                    " << "edge:" << ah2.h.edge->id() << "; side:" <<  ah2.h.side << "; angle:" << ah2.angle;
                ///////////////////////////////

                // computing what to draw
                Eigen::Vector2d v1(ah1.u[1],-ah1.u[0]);
                Eigen::Vector2d v2(ah2.u[1],-ah2.u[0]);
                Eigen::Vector2d p = pos();
                Eigen::Vector2d p1 = p - ah1.r * v1;
                Eigen::Vector2d p2 = p + ah2.r * v2;

                // get colors
                double r1=0, g1=0, b1=0, a1=1;
                double r2=0, g2=0, b2=0, a2=1;
                double r, g, b, a;
                if(!drawBlackAndWhite)
                {
                    ah1.h.edge->getColor(&r1,&g1,&b1,&a1);
                    ah2.h.edge->getColor(&r2,&g2,&b2,&a2);
                }
                r = 0.5*(r1+r2);
                g = 0.5*(g1+g2);
                b = 0.5*(b1+b2);
                a = 0.5*(a1+a2);

                // different line cap style
                if( flatStyle || ( doThreshold && (dAngle > 2*pi - threshold || dAngle < pi + threshold) ))
                {
                    // draw triangle
                    glBegin(GL_TRIANGLES);
                    {
                        glColor4d(r,g,b,a);
                        glVertex2d(p[0],p[1]);
                        glColor4d(r2,g2,b2,a2);
                        glVertex2d(p2[0],p2[1]);
                        glColor4d(r1,g1,b1,a1);
                        glVertex2d(p1[0],p1[1]);
                    }
                    glEnd();
                }
                else
                {
                    Eigen::Matrix2d A;
                    A << ah2.u[0], - ah1.u[0],
                         ah2.u[1], - ah1.u[1];
                    double epsilon = 1e-6;
                    double det = A.determinant();
                    if(det > epsilon || det < -epsilon)
                    {
                        // compute intersection
                        Eigen::Vector2d rightHandSide(p1[0]-p2[0],p1[1]-p2[1]);
                        Eigen::Vector2d sol = A.inverse() * rightHandSide;
                        Eigen::Vector2d q = p2 + sol[0]*ah2.u;

                        // draw quad
                        glBegin(GL_POLYGON);
                        {
                            glColor4d(r,g,b,a);
                            glVertex2d(p[0],p[1]);
                            glColor4d(r2,g2,b2,a2);
                            glVertex2d(p2[0],p2[1]);
                            glColor4d(r,g,b,a);
                            glVertex2d(q[0],q[1]);
                            glColor4d(r1,g1,b1,a1);
                            glVertex2d(p1[0],p1[1]);
                        }
                        glEnd();
                    }
                }
            }
        }
    }
    */
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
    foreach(KeyEdge * keyEdge, keyEdges)
    {
        if(keyEdge->startVertex()->toVertexCell() == this)
            res << Halfedge(keyEdge, true);
        if(keyEdge->endVertex()->toVertexCell() == this)
            res << Halfedge(keyEdge, false);
    }
    foreach(InbetweenEdge * inbetweenEdge, inbetweenEdges)
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

}
