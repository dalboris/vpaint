// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef VAC_KEYHALFEDGE_H
#define VAC_KEYHALFEDGE_H

#include "HalfedgeBase.h"
#include "KeyEdge.h"

#include "EdgeSample.h"

namespace VectorAnimationComplex
{

class KeyHalfedge: public HalfedgeBase<KeyEdge>
{
public:
    // Constructors
    KeyHalfedge();
    KeyHalfedge(KeyEdge * e, bool s);

    // Downcasting
    KeyHalfedge(const HalfedgeBase<KeyEdge> & base);
    KeyHalfedge(const Halfedge & base);

    // Other methods
    bool isSplittedLoop() const;


    KeyVertex * startVertex() const;
    KeyVertex * endVertex() const;
    Time time() const;
    double length();
    Eigen::Vector2d pos(double s);
    EdgeSample sample(double s);
    Eigen::Vector2d leftPos();
    Eigen::Vector2d rightPos();
    Eigen::Vector2d leftDer();
    Eigen::Vector2d rightDer();
    QList<KeyHalfedge> sorted(const QList<KeyHalfedge> & adj);

    // finding incident halfedges
    QList<KeyHalfedge> endIncidentHalfEdges();
    KeyHalfedge next();
    //QList<KeyHalfEdge> startIncidentHalfEdges();
    //KeyHalfEdge previous();
};

struct KeyAngleHalfEdge
{
    KeyAngleHalfEdge(KeyHalfedge h, double a) : he(h), angle(a) {}
    KeyHalfedge he;
    double angle;
    bool operator<(const KeyAngleHalfEdge & other) const
        {return angle < other.angle;}
};

}

#endif // VAC_KEYHALFEDGE_H
