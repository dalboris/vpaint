// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef VAC_INTERSECTION_H
#define VAC_INTERSECTION_H

#include <QList>
#include "Eigen.h"

namespace VectorAnimationComplex
{
class KeyVertex;
class KeyEdge;
class IntersectionList;

// Compute the intersection between two segments AB and CD
//
// Their length is assumed to be less or equal than ds
// Return true if they interesect, false otherwise
// in addition, returns in s and t the position of intersection:
//   I = A + s*(B-A)/||B-A|| = C + t*(D-C)/||D-C||
bool segmentIntersection(
    const Eigen::Vector2d & a, const Eigen::Vector2d & b,
    const Eigen::Vector2d & c, const Eigen::Vector2d & d,
    double dsSquared, double &s, double &t);

// convenient structure to store intersections between objects
struct Intersection
{
    Intersection(double s0, double t0, KeyVertex * n0 = 0) :
        type(SELF), s(s0), t(t0), vertex(n0), removed(false) {}

    enum Type { SELF, EDGE, ANIMEDGE };

    Type type;
    double s;
    double t;
    KeyVertex * vertex;
    bool removed;

    bool operator< (const Intersection & other) const {
            return s < other.s;}
    static bool compareT (const Intersection & e1, const Intersection & e2) {
            return e1.t < e2.t;}
    static bool pLessThanS( Intersection * const & i1, Intersection* const & i2)
        {return (*i1) < (*i2);}
    static bool pLessThanT( Intersection * const & i1, Intersection* const & i2)
        {return i1->t < i2->t;}
};

// intersection between a stroke and an edge
struct EdgeInter: public Intersection
{
    EdgeInter(KeyEdge * e0, double s0, double t0) :
        Intersection(s0,t0), edge(e0) { type = EDGE; }

    KeyEdge * edge;
};

// remove from a list all intersections marked "removed"
class IntersectionList: public QList<Intersection*>
{
public:

    void sort() {qSort(begin(), end(), Intersection::pLessThanS);}
    void sortT() {qSort(begin(), end(), Intersection::pLessThanT);}

    void clean()
    {
        IntersectionList aux;
        for(int i=0; i<size(); i++)
        {
            Intersection* inter = at(i);
            if(!inter->removed)
                aux << inter;
        }
        *this = aux;
    }
};
    
}


    

#endif
