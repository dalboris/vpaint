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

#include "SmartKeyEdgeSet.h"

#include <QStack>

namespace VectorAnimationComplex
{

/////////////////////////////////////////////////////////////////////
//            Single Connected Component computation               //

SmartConnectedKeyEdgeSet::SmartConnectedKeyEdgeSet(const KeyEdgeSet & edgeSet):
    edgeSet_(edgeSet),
    path_(edgeSet),
    loop_(edgeSet),
    hole_(edgeSet)
{
}

SmartConnectedKeyEdgeSet::EdgeSetType SmartConnectedKeyEdgeSet::type() const
{
    KeyEdge * singleEdge = edge();

    if(edgeSet_.size() == 0)
    {
        return EMPTY;
    }
    else if(singleEdge)
    {
        if(singleEdge->isClosed())
        {
            return CLOSED_EDGE;
        }
        else if(singleEdge->isSplittedLoop())
        {
            return OPEN_EDGE_LOOP;
        }
        else
        {
            return OPEN_EDGE_PATH;
        }
    }
    else if(path_.isValid())
    {
        return SIMPLE_PATH;
    }
    else if(loop_.isValid())
    {
        return SIMPLE_LOOP;
    }
    else if(hole_.isValid())
    {
        return PATH_LOOP_DECOMPOSITION;
    }
    else
    {
        return GENERAL;
    }
}

KeyEdge * SmartConnectedKeyEdgeSet::edge() const
{
    if(edgeSet_.size() == 1)
    {
        return *edgeSet_.begin();
    }
    else
    {
        return 0;
    }
}

ProperPath SmartConnectedKeyEdgeSet::path() const
{
    return path_;
}

ProperCycle SmartConnectedKeyEdgeSet::loop() const
{
    return loop_;
}

CycleHelper SmartConnectedKeyEdgeSet::hole() const
{
    return hole_;
}

KeyEdgeSet SmartConnectedKeyEdgeSet::edgeSet() const
{
    return edgeSet_;
}


/////////////////////////////////////////////////////////////////////
//           Multiple Connected Component computation              //


SmartKeyEdgeSet::SmartKeyEdgeSet(const KeyEdgeSet & edgeSetConst) :
    edgeSet_(edgeSetConst)
{
    // ----- Compute connected components -----

    // This variable store the edges that have not yet been assigned a connected component
    KeyEdgeSet remainingEdges = edgeSet_;


    // ===== First, each closed edge is its own connected component =====

    KeyEdgeSet remainingEdgesCopy = remainingEdges;
    for(KeyEdge * iedge: remainingEdgesCopy)
    {
        if(iedge->isClosed())
        {
            remainingEdges.remove(iedge);
            KeyEdgeSet connectedEdges;
            connectedEdges << iedge;
            connectedComponents_ << SmartConnectedKeyEdgeSet(connectedEdges);
        }
    }


    // ===== Define the 1-subcomplex made of the remaining vertices and open edges =====

    // Data structure for cells of the original complex
    typedef KeyVertex Vertex;
    typedef KeyEdge Edge;

    // Data structure for cells of the subcomplex
    struct SubVertex;
    struct SubEdge
    {
        SubEdge(Edge * edge) : edge(edge), leftSubVertex(0), rightSubVertex(0), marked(false) {}

        // Edge from SubEdge
        Edge * edge;

        // Boundary SubVertices
        SubVertex * leftSubVertex;
        SubVertex * rightSubVertex;

        // Has the edge already been visited
        bool marked;
    };
    struct SubVertex
    {
        SubVertex(Vertex * vertex) : vertex(vertex), subEdges() {}

        // Vertex from SubVertex
        Vertex * vertex;

        // Star SubEdges
        QSet<SubEdge*> subEdges;
    };

    // Populate subcomplex
    QSet<SubEdge*> subEdges;
    QSet<SubVertex*> subVertices;
    QMap<Vertex*, SubVertex*> vertexToSubVertex;
    QMap<Edge*, SubEdge*> edgeToSubEdge;
    for(Edge * edge: remainingEdges)
    {
        // create new subcomplex edge
        SubEdge * subEdge = new SubEdge(edge);
        subEdges << subEdge;
        edgeToSubEdge[edge] = subEdge;

        // assign its left subcomplex vertex (create it if doesn't exist yet)
        Vertex * startVertex = edge->startVertex(); // not NULL because edge is open
        SubVertex * leftSubVertex = vertexToSubVertex.value(startVertex, 0); // returns 0 if not found
        if(!leftSubVertex)
        {
            leftSubVertex = new SubVertex(startVertex);
            subVertices << leftSubVertex;
            vertexToSubVertex[startVertex] = leftSubVertex;
        }
        subEdge->leftSubVertex = leftSubVertex;
        leftSubVertex->subEdges << subEdge;

        // assign its right subcomplex vertex (create it if doesn't exist yet)
        Vertex * endVertex = edge->endVertex(); // we know it's not null since we rejected pure loops
        SubVertex * rightSubVertex = vertexToSubVertex.value(endVertex, 0); // returns 0 if not found
        if(!rightSubVertex)
        {
            rightSubVertex = new SubVertex(endVertex);
            subVertices << rightSubVertex;
            vertexToSubVertex[endVertex] = rightSubVertex;
        }
        subEdge->rightSubVertex = rightSubVertex;
        rightSubVertex->subEdges << subEdge;
    }

    // ===== Compute connected components of subcomplex =====

    while(!remainingEdges.isEmpty())
    {
        // New connected component
        KeyEdgeSet connectedEdges;

        // get the first edge/subEdge of a new connected component
        Edge * firstEdge = *remainingEdges.begin();
        SubEdge * firstSubEdge =    edgeToSubEdge[firstEdge];

        // get all subEdges/edges connected to this sub/edge/edge
        QStack<SubEdge *> toProcess;
        toProcess.push(firstSubEdge);
        while(!toProcess.isEmpty())
        {
            SubEdge * subEdge = toProcess.top();
            toProcess.pop();

            // marked means:
            //  - it has already been removed from remainingEdges
            //  - it has already been added to connectedEdges
            //  - all its neighbours have already been inserted in toProcess (possibly twice)

            if(!subEdge->marked)
            {
                // Mark and remove
                remainingEdges.remove(subEdge->edge);
                connectedEdges.insert(subEdge->edge);
                subEdge->marked = true;

                // Insert neighbours in stack
                for(SubEdge * neighbour: subEdge->leftSubVertex->subEdges)
                    toProcess.push(neighbour);
                for(SubEdge * neighbour: subEdge->rightSubVertex->subEdges)
                    toProcess.push(neighbour);
            }
        }

        // Process connected component
        connectedComponents_ << SmartConnectedKeyEdgeSet(connectedEdges);
    }


    // ===== Release memory =====

    // release memory
    for(SubEdge * subEdge: subEdges)
        delete subEdge;
    for(SubVertex * subVertex: subVertices)
        delete subVertex;
}

int SmartKeyEdgeSet::numConnectedComponents() const
{
    return connectedComponents_.size();
}

SmartConnectedKeyEdgeSet & SmartKeyEdgeSet::operator [] (int i)
{
    return connectedComponents_[i];
}


}
