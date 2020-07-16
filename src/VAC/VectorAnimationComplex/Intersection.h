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
class EdgeInter: public Intersection
{
public:
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
