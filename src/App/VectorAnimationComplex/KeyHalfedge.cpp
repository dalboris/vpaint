// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "KeyHalfedge.h"

#include "../GeometryUtils.h"
#include "KeyVertex.h"
#include "EdgeGeometry.h"

namespace VectorAnimationComplex
{

// Other methods
bool KeyHalfedge::isSplittedLoop() const
{
    if(edge) return edge->isSplittedLoop();
    else return false;
}


KeyHalfedge::KeyHalfedge() :
    HalfedgeBase<KeyEdge>()
{
}

KeyHalfedge::KeyHalfedge(KeyEdge * e, bool s) :
    HalfedgeBase<KeyEdge>(e,s)
{
}

KeyHalfedge::KeyHalfedge(const HalfedgeBase<KeyEdge> & base) :
    HalfedgeBase<KeyEdge>(base)
{
}

KeyHalfedge::KeyHalfedge(const Halfedge & base) :
    HalfedgeBase<KeyEdge>(HalfedgeImpl::edgeFromCell<KeyEdge>(base.edge), base.side)
{
}

KeyVertex * KeyHalfedge::startVertex() const
{
    if(edge)
    {
        if(side) return edge->startVertex();
        else return edge->endVertex();
    }
    else
        return 0;
}
KeyVertex * KeyHalfedge::endVertex() const
{
    if(edge)
    {
        if(side) return edge->endVertex();
        else return edge->startVertex();
    }
    else
        return 0;
}

Time KeyHalfedge::time() const
{
    if(edge)
    {
        return edge->time();
    }
    else
        return Time();
}

KeyHalfedge KeyHalfedge::next()
{
    if(!edge)
        return KeyHalfedge();

    QList<KeyHalfedge> halfedges = endIncidentHalfEdges();

    // compute the angles between this halfedge and the others
    Eigen::Vector2d u = - rightDer();
    QList<double> angles;
    for(int i=0; i<halfedges.size(); i++) {
        double angle = GeometryUtils::angleLike(u, halfedges[i].leftDer());
        angles << angle;
    }

    // take the smallest
    KeyHalfedge res = opposite();
    double minAngle = 5;
    for(int i=0; i<angles.size(); i++) {
        if(angles[i]<minAngle)
        {
            minAngle = angles[i];
            res = halfedges[i];
        }
    }

    return res;
}

QList<KeyHalfedge> KeyHalfedge::endIncidentHalfEdges()
{
    KeyVertex * v = endVertex();
    KeyEdgeSet edges = v->star();
    QList<KeyHalfedge> halfedges;
    foreach(KeyEdge * e, edges)
    {
        if(e->startVertex() == v)
            halfedges << KeyHalfedge(e,true);
        if(e->endVertex() == v)
            halfedges << KeyHalfedge(e,false);
    }
    halfedges.removeOne(KeyHalfedge(edge,!side));
    return halfedges;
}

Eigen::Vector2d KeyHalfedge::rightDer()
{
    if(!edge)
        return Eigen::Vector2d(1,0);

    if(side)
        return edge->geometry()->der(edge->geometry()->length());
    else
        return - edge->geometry()->der(0);
}
Eigen::Vector2d KeyHalfedge::leftDer()
{
    if(!edge)
        return Eigen::Vector2d(1,0);

    if(side)
        return edge->geometry()->der(0);
    else
        return - edge->geometry()->der(edge->geometry()->length());
}

QList<KeyHalfedge> KeyHalfedge::sorted(const QList<KeyHalfedge> & adj)
{
    if(!edge)
        return QList<KeyHalfedge>();

    // assume no adj is null
    // the he in adj verify:
    // adj[i].startVertex() == this.endVertex()

    // compute all the angles between this he to the others
    Eigen::Vector2d u = - rightDer();
    QList<KeyAngleHalfEdge> list;
    foreach(KeyHalfedge he, adj)
    {
        double angle = GeometryUtils::angleLike(u, he.leftDer());
        list << KeyAngleHalfEdge(he, angle);
    }

    // sort the he according to this angle
    qSort(list);

    // return the he sorted
    QList<KeyHalfedge> res;
    foreach(KeyAngleHalfEdge ahe, list)
        res << ahe.he;
    return res;
}

double KeyHalfedge::length()
{
    if(!edge)
        return 0;

    return edge->geometry()->length();
}

Eigen::Vector2d KeyHalfedge::pos(double s)
{
    if(!edge)
        return Eigen::Vector2d(0,0);

    if(side)
        return edge->geometry()->pos2d(s);
    else
        return edge->geometry()->pos2d(length()-s);
}
EdgeSample KeyHalfedge::sample(double s)
{
    if(!edge)
        return EdgeSample();

    if(side)
        return edge->geometry()->pos(s);
    else
        return edge->geometry()->pos(length()-s);
}



Eigen::Vector2d KeyHalfedge::leftPos()
{
    if(!edge)
        return Eigen::Vector2d(0,0);

    if(side)
        return edge->geometry()->pos2d(0);
    else
        return edge->geometry()->pos2d(length());
}

Eigen::Vector2d KeyHalfedge::rightPos()
{
    if(!edge)
        return Eigen::Vector2d(0,0);

    if(side)
        return edge->geometry()->pos2d(length());
    else
        return edge->geometry()->pos2d(0);
}

}
