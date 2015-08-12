// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

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
        foreach(Cell * c, addedCells)
        {
            CellSet neighbourhood = c->neighbourhood();
            foreach(Cell * d, neighbourhood)
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
    foreach(Cell * b, c->boundary())
        res << b;
    return res;
}

CellSet closure(const CellSet & cells)
{
    CellSet res;
    foreach(Cell * c, cells)
    {
        res << c;
        foreach(Cell * b, c->boundary())
            res << b;
    }
    return res;
}

CellSet fullstar(Cell * c)
{
    CellSet res;
    res << c;
    foreach(Cell * b, c->star())
        res << b;
    return res;
}

CellSet fullstar(const CellSet & cells)
{
    CellSet res;
    foreach(Cell * c, cells)
    {
        res << c;
        foreach(Cell * b, c->star())
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
    foreach(KeyEdge * edge, cells)
        isMarked[edge] = false;

    QMap<KeyEdge *, int> component;
    foreach(KeyEdge * edge, cells)
        component[edge] = -1;

    int numComponents = 0;


    // Execute algorithm

    foreach(KeyEdge * edge, cells)
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
                foreach(KeyEdge * other, cells)
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

    foreach(KeyEdge * edge, cells)
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
