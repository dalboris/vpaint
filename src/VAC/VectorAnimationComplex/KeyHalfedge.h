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
