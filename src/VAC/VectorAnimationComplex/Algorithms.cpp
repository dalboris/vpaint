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

#include "Algorithms.h"
#include "Cell.h"
#include "KeyEdge.h"

#include <QStack>
#include <QMap>

namespace VectorAnimationComplex
{

namespace Algorithms
{

CellSet connected(const CellSet & cells)
{
    CellSet res = cells;
    CellSet addedCells = cells;

    // while some cells have been added
    while(addedCells.size() != 0)
    {
        CellSet newAddedCells;
        for(Cell * c: addedCells)
        {
            CellSet neighbourhood = c->neighbourhood();
            for(Cell * d: neighbourhood)
            {
                if(!res.contains(d))
                {
                    res << d;
                    newAddedCells << d;
                }
            }
        }
        addedCells = newAddedCells;
    }

    return res;
}


CellSet closure(Cell * c)
{
    CellSet res;
    res << c;
    const auto& boundary = c->boundary();
    for(Cell * b: boundary)
        res << b;
    return res;
}

CellSet closure(const CellSet & cells)
{
    CellSet res;
    for(Cell * c: cells)
    {
        res << c;
        const auto& boundary = c->boundary();
        for(Cell * b: boundary)
            res << b;
    }
    return res;
}

CellSet fullstar(Cell * c)
{
    CellSet res;
    res << c;
    const auto& star = c->star();
    for(Cell * b: star)
        res << b;
    return res;
}

CellSet fullstar(const CellSet & cells)
{
    CellSet res;
    for(Cell * c: qAsConst(cells))
    {
        res << c;
        const auto& star = c->star();
        for(Cell * b: star)
            res << b;
    }
    return res;
}

// decompose `cells` in a list of connected, mutually disconnected, cells
QList<KeyEdgeSet> connectedComponents(const KeyEdgeSet & cells)
{
    // ---- Detect connected components ----

    // Initialize data structure for algorithm

    QMap<KeyEdge *, bool> isMarked;
    for(KeyEdge * edge: cells)
        isMarked[edge] = false;

    QMap<KeyEdge *, int> component;
    for(KeyEdge * edge: cells)
        component[edge] = -1;

    int numComponents = 0;


    // Execute algorithm

    for(KeyEdge * edge: cells)
    {
        // if has already been assigned a connected component, do nothing
        if(!isMarked[edge])
        {
            // create a new connected component
            numComponents++;

            // create the stack of unvisited but marked edges to visit
            QStack<KeyEdge *> stack;

            // add the first edge of connected component as marked
            isMarked[edge] = true;
            stack.push(edge);

            // While there are edges to visit
            while(!stack.isEmpty())
            {
                // get unvisited edge
                KeyEdge * edgeToVisit = stack.pop();

                // Note: edgeToVisit is already marked.

                // visit edge:
                //  1) assign it a connected component
                //  2) find other edges to visit

                // Assign connected component
                component[edgeToVisit] = numComponents-1;

                //  Find other edges to visit
                for(KeyEdge * other: cells)
                {
                    if(!isMarked[other]) // note: this exlude other == edgeToVisit
                    {
                        if(areIncident(edgeToVisit,other))
                        {
                            // mark other, insert it in stack
                            isMarked[other] = true;
                            stack.push(other);
                        }
                    }
                }
            }
        }
    }


    // ---- Convert to output ----

    QList<KeyEdgeSet> res;
    for(int i=0; i<numComponents; ++i)
        res << KeyEdgeSet();

    for(KeyEdge * edge: cells)
        res[component[edge]] << edge;

    return res;
}

bool areIncident(const KeyEdge * e1, const KeyEdge * e2)
{
    if(!e1 || !e2)
        return false;

    if(e1 == e2)
        return true;

    if(e1->isClosed())
        return false;

    if( (e1->startVertex() == e2->startVertex()) ||
        (e1->startVertex() == e2->endVertex()) ||
        (e1->endVertex() == e2->startVertex()) ||
        (e1->endVertex() == e2->endVertex()) )
    {
        return true;
    }
    else
    {
        return false;
    }
}


}

}
