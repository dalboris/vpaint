// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef SMARTINSTANTEDGESET_H
#define SMARTINSTANTEDGESET_H

#include "KeyEdge.h"
#include "ProperPath.h"
#include "ProperCycle.h"
#include "CycleHelper.h"
#include "CellList.h"

namespace VectorAnimationComplex
{

// Assumes edgeSet is connected
class SmartConnectedKeyEdgeSet
{
public:
    SmartConnectedKeyEdgeSet(const KeyEdgeSet & edgeSet);

    // Get type
    enum EdgeSetType {
        EMPTY,

        CLOSED_EDGE, // A single closed edge

        OPEN_EDGE_PATH, // A single open edge with start() != end()
        OPEN_EDGE_LOOP, // A single open edge with start() == end()

        SIMPLE_PATH, // N >= 2 consecutive halfedges with h[0].start() != h[N-1].end()
                     //                                   (i != j) => (h[i].edge() != h[j].edge())
                     //                                   (i != j) => (h[i].start() != h[j].start())
        SIMPLE_LOOP, // N >= 2 consecutive halfedges with h[0].startVertex() == h[N-1].endVertex()
                     //                                   (i != j) => (h[i].edge() != h[j].edge())
                     //                                   (i != j) => (h[i].start() != h[j].start())

        PATH_LOOP_DECOMPOSITION, // None of the above, but the edges can be partitionned
                                 // into simple paths and simple loops such that its intersection
                                 // graph is a tree. The intersection graph is defined as:
                                 //   - each (edge-disjoint) path or loop is a node
                                 //   - each pair of nodes n1 and n2 are connected by exactly K edges,
                                 //     where K is the number of vertices in the intersection between n1 and n2

        GENERAL // None of the above
    };
    EdgeSetType type() const;

    // Get decomposition

    // Edge
    // If type() is not one of:
    //  - CLOSED_EDGE
    //  - OPEN_EDGE_PATH
    //  - OPEN_EDGE_LOOP
    // Then it returns a NULL pointer
    KeyEdge * edge() const;

    // Simple path
    // If type() is not one of:
    //  - OPEN_EDGE_PATH
    //  - SIMPLE_PATH
    // Then it returns an invalid path
    ProperPath path() const;

    // Simple loop
    // If type() is not one of:
    //  - CLOSED_EDGE
    //  - OPEN_EDGE_LOOP
    //  - SIMPLE_LOOP
    // Then it returns an invalid loop
    ProperCycle loop() const;

    // Path-loop decomposition
    // If type() is not one of:
    //  - CLOSED_EDGE
    //  - OPEN_EDGE_PATH
    //  - OPEN_EDGE_LOOP
    //  - SIMPLE_PATH
    //  - SIMPLE_LOOP
    //  - PATH_LOOP_DECOMPOSITION
    // Then it returns an invalid hole
    CycleHelper hole() const;

    // In any cases, you can still get the original set of edges
    KeyEdgeSet edgeSet() const;

private:
    KeyEdgeSet edgeSet_;
    ProperPath path_;
    ProperCycle loop_;
    CycleHelper hole_;
};

class SmartKeyEdgeSet
{
public:
    SmartKeyEdgeSet(const KeyEdgeSet & edgeSet);

    int numConnectedComponents() const;
    SmartConnectedKeyEdgeSet & operator [] (int i);

private:
    KeyEdgeSet edgeSet_;
    QList<SmartConnectedKeyEdgeSet> connectedComponents_;
};

}

#endif // SMARTINSTANTEDGESET_H
