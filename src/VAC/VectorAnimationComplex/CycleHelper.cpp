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

#include "CycleHelper.h"

#include "KeyVertex.h"
#include "KeyEdge.h"
#include "VAC.h"

#include "../SaveAndLoad.h"

#include <QtDebug>
#include <QMessageBox>
#include <QStack>

namespace VectorAnimationComplex
{

// Invalid hole
CycleHelper::CycleHelper() :
    singleVertex_(0)
{
}

// Create a hole that is a single vertex
CycleHelper::CycleHelper(KeyVertex * vertex) :
    singleVertex_(vertex)
{
    //qDebug() << "creating a vertex hole";
}

// create a hole that is not a single vertex
CycleHelper::CycleHelper(const KeyEdgeSet & edgeSetConst) :
    singleVertex_(0)
{
    //qDebug() << "creating a paths/loops hole";

    // If no edge, then invalid
    if(edgeSetConst.isEmpty())
        return;

    // if not all edges at same time, then invalid
    KeyEdge * first = *edgeSetConst.begin();
    Time t = first->time();
    for(KeyEdge * iedge: edgeSetConst)
    {
        if(iedge->time() != t)
        {
            //QMessageBox::information(0, QObject::tr("operation aborted"),
            //                         QObject::tr("not all edges are on same time"));
            return;
        }
    }

    // copy the set to be able to modify it
    KeyEdgeSet edgeSet = edgeSetConst;

    // check case where it's a pure loop
    if(first->isClosed())
    {
        if(edgeSet.size() != 1)
        {
            //QMessageBox::information(0, QObject::tr("operation aborted"),
            //                         QObject::tr("more than one edge and one of them is a pure loop"));
            return;
        }
        else
        {
            ProperCycle loop(edgeSet);
            if(loop.isValid())
            {
                loops_ << loop;
            }
            else
            {

                //QMessageBox::information(0, QObject::tr("operation aborted"),
                //                         QObject::tr("hole seemed to be a pure loop, but couldn't create the loop"));
                return;
            }
        }
    }
    else
    {
        // ==========  MAIN ALGORITHM  ==========

        // defining the subcomplex

        struct Vertex;
        struct Edge
        {
            Edge(KeyEdge * edge) :
                edge(edge),
                leftNode(0),
                rightNode(0),
                marked(false) {}

            // corresponding cell
            KeyEdge * edge;

            // incident nodes
            Vertex * leftNode;
            Vertex * rightNode;

            // for testing if subcomplex is connected
            bool marked;
        };
        struct Vertex
        {
            Vertex(KeyVertex * vertex) :
                vertex(vertex),
                edges(),
                parent(0),
                isRoot(false),
                visited(false) {}

            // corresponding cell
            KeyVertex * vertex;

            // incident edges
            QSet<Edge*> edges;

            // tree relationship
            Edge * parent;
            bool isRoot;
            bool visited;
            Vertex * parentNode() { if(parent->leftNode == this) return parent->rightNode; else return parent->leftNode; }
        };

        // populate the subcomplex
        QSet<Edge*> subcomplexEdges;
        QSet<Vertex*> subcomplexNodes;
        QMap<KeyVertex*, Vertex*> vertexToNode;
        // /!\ be careful of memory leaks from here

        for(KeyEdge * iedge: edgeSet)
        {
            // detect pure loop here, and abort if any
            if(iedge->isClosed())
            {
                //QMessageBox::information(0, QObject::tr("operation aborted"),
                //                         QObject::tr("more than one edge and one of them is a pure loop"));

                for(Edge * edge: subcomplexEdges)
                    delete edge;
                for(Vertex * node: subcomplexNodes)
                    delete node;

                return;
            }

            // create new subcomplex edge
            Edge * edge = new Edge(iedge);
            subcomplexEdges << edge;

            // find its left subcomplex node
            KeyVertex * startVertex = iedge->startVertex(); // we know it's not null since we rejected pure loops
            Vertex * leftNode = vertexToNode.value(startVertex, 0); // returns 0 if not found
            if(!leftNode)
            {
                leftNode = new Vertex(startVertex);
                subcomplexNodes << leftNode;
                vertexToNode[startVertex] = leftNode;
            }
            edge->leftNode = leftNode;
            leftNode->edges << edge;

            // find its right subcomplex node
            KeyVertex * endVertex = iedge->endVertex(); // we know it's not null since we rejected pure loops
            Vertex * rightNode = vertexToNode.value(endVertex, 0); // returns 0 if not found
            if(!rightNode)
            {
                rightNode = new Vertex(endVertex);
                subcomplexNodes << rightNode;
                vertexToNode[endVertex] = rightNode;
            }
            edge->rightNode = rightNode;
            rightNode->edges << edge;
        }

        // Check that the subcomplex is connected
        Edge * firstEdge = *subcomplexEdges.begin();
        QStack<Edge *> toProcess;
        firstEdge->marked = true;
        toProcess.push(firstEdge);
        while(!toProcess.isEmpty())
        {
            Edge * edge = toProcess.top();
            toProcess.pop();

            for(Edge * neighbour: edge->leftNode->edges)
            {
                if(!neighbour->marked)
                {
                    neighbour->marked = true;
                    toProcess.push(neighbour);
                }
            }
            for(Edge * neighbour: edge->rightNode->edges)
            {
                if(!neighbour->marked)
                {
                    neighbour->marked = true;
                    toProcess.push(neighbour);
                }
            }
        }
        for(Edge * edge: subcomplexEdges)
        {
            if(!edge->marked)
            {
                //QMessageBox::information(0, QObject::tr("operation aborted"),
                //                         QObject::tr("the selected edges are not all connected together"));

                for(Edge * edge: subcomplexEdges)
                    delete edge;
                for(Vertex * node: subcomplexNodes)
                    delete node;

                return;
            }
        }

        // store num of vertices for later, before we start removing them
        int numVertices = subcomplexNodes.size();


        // find loops
        while (!subcomplexEdges.isEmpty())
        {
            //  initialization
            for(Vertex * node: subcomplexNodes)
            {
                node->parent = 0;
                node->isRoot = false;
                node->visited = false;
            }

            bool loopFound = false;

            while(true) // break either when a loop is found, or all vertices have been visited
            {
                // find a node that has not been visited yet, if any
                Vertex * notYetVisitedNode = 0;
                for(Vertex * node: subcomplexNodes)
                {
                    if(!node->visited)
                    {
                        notYetVisitedNode = node;
                        break;
                    }
                }

                // break if all nodes have already been visited
                if(!notYetVisitedNode)
                    break; // if we are here, then it means loopFound == false

                // otherwise, the the unvisited node as the root
                notYetVisitedNode->isRoot = true;

                // traverse the VAC until a loop is found, if any
                QStack<Vertex*> stack;
                stack.push(notYetVisitedNode);
                while(!stack.isEmpty())
                {
                    Vertex* node = stack.top();
                    stack.pop();
                    node->visited = true;

                    // depth-first recursive call: check all children
                    for(Edge * incidentEdge: node->edges)
                    {
                        if(incidentEdge != node->parent) // do not check parent
                        {
                            Vertex * child = incidentEdge->leftNode;
                            if(child == node)
                                child = incidentEdge->rightNode; // note: this may still be equal to node,
                                                                 // but it's not an issue for the algorithm

                            // check if the child has already been visited or not
                            if(child->visited) // caution, can be non visited but already on the stack,
                                               // and then already has a parent
                            {
                                // we've found our loop!

                                // compute all edges involved in the loop
                                QList<Edge*> edgesInLoop;
                                edgesInLoop << incidentEdge;
                                while(node != child)
                                {
                                    edgesInLoop << node->parent; // normally, node can't be root
                                    node = node->parentNode();
                                }

                                // remove them from the subcomplex and add them to the list
                                // of instant edges in the loop
                                KeyEdgeSet iedgesInLoop;
                                for(Edge * edge: edgesInLoop)
                                {
                                    // disconnect edge from left and right vertices
                                    Vertex * leftNode = edge->leftNode;
                                    Vertex * rightNode = edge->rightNode;
                                    leftNode->edges.remove(edge);
                                    if(rightNode != leftNode)
                                        rightNode->edges.remove(edge);

                                    // if vertices become isolated, remove them from subcomplex too
                                    if(leftNode->edges.size() == 0)
                                    {
                                        subcomplexNodes.remove(leftNode);
                                        delete leftNode;
                                    }
                                    if(rightNode != leftNode && rightNode->edges.size() == 0)
                                    {
                                        subcomplexNodes.remove(rightNode);
                                        delete rightNode;
                                    }

                                    // remove edge from subcomplex
                                    subcomplexEdges.remove(edge);
                                    iedgesInLoop << edge->edge;
                                    delete edge;
                                }

                                // add loop to final loops
                                loops_ << ProperCycle(iedgesInLoop);
                                loopFound = true;
                                //qDebug() << "loop found";
                                break; // this breaks the loop over all children

                            }
                            else // if child was not visited yet
                            {
                                // Note: child might already be in the stack
                                //       but we don't care. we change its parent
                                //       and add it again
                                child->parent = incidentEdge;
                                stack.push(child);
                            } // end if

                        } // end if used not to recurse "up"

                    } // end for: all children added to stack

                    if(loopFound)
                        break; // this break the loop over this connected component

                } // end while(!stack.isEmpty()): the whole connected component has been checked for loops

                if(loopFound)
                    break; // this break the loop over all connected component

            } // end while loop (over all connected components) to find a loop



            if(!loopFound)
                break; // this break out of the while loop searching for loops.


        } // no more loops

        // find paths
        // for now, very trivial method: each remaining edge becomes a path
        for(Edge * edge: subcomplexEdges)
        {
            KeyCellSet set;
            set << edge->edge;
            paths_ << ProperPath(set);
            //qDebug() << "path found";
        }

        // release memory
        for(Edge * edge: subcomplexEdges)
            delete edge;
        for(Vertex * node: subcomplexNodes)
            delete node;

        // ---- Now, check that the hole is valid ----

        // cf p 271.
        int eulerCharacteristic = 1;
        for(int i=0; i<loops_.size(); ++i)
            eulerCharacteristic += loops_[i].size();     // num vertices in loop
        for(int i=0; i<paths_.size(); ++i)
            eulerCharacteristic += paths_[i].size() + 1; // num vertices in path
        eulerCharacteristic -= numVertices;
        eulerCharacteristic -= loops_.size() + paths_.size();
        if(eulerCharacteristic != 0)
        {
            //QMessageBox::information(0, QObject::tr("operation aborted"),
            //                         QObject::tr("the hole is not valid: the selected edges can't be "
            //                                     "decomposed in simple loops and paths forming a non-cyling network."));
            loops_.clear();
            paths_.clear();

            return;
        }

    }



}

// type of the hole
bool CycleHelper::isValid() const { return isSingleVertex() || nLoops() || nPaths(); }
bool CycleHelper::isSingleVertex() const { return singleVertex_; }

// Time
Time CycleHelper::time() const
{
    if(isSingleVertex())
        return singleVertex_->time();
    else if(nLoops())
        return loops_[0].time();
    else
        return paths_[0].time();
}

// In case of single vertex (can be call a Steiner vertex)
KeyVertex * CycleHelper::vertex() const { return singleVertex_; }

// In case of not a single vertex
int CycleHelper::nLoops() const { return loops_.size(); }
const ProperCycle & CycleHelper::loop(int i) const { return loops_[i]; }
int CycleHelper::nPaths() const { return paths_.size(); }
const ProperPath & CycleHelper::path(int i) const { return paths_[i]; }


// The set of cells this helper points to
KeyCellSet CycleHelper::cells() const
{
    KeyCellSet res;
    if(isSingleVertex())
    {
        res << vertex();
    }
    else
    {
        for(int i=0; i<nLoops(); ++i)
        {
            for(int j=0; j<loop(i).size(); ++j)
            {
                if(loop(i)[j].startVertex())
                    res << loop(i)[j].startVertex();
                res << loop(i)[j].edge;
            }
        }
        for(int i=0; i<nPaths(); ++i)
        {
            res << path(i)[0].startVertex();
            for(int j=0; j<path(i).size(); ++j)
            {
                res << path(i)[j].edge;
                res << path(i)[j].endVertex();
            }
        }
    }
    return res;
}


// serialization and copy
void CycleHelper::remapPointers(VAC * newVAC)
{
    if(singleVertex_)
        singleVertex_ = newVAC->getCell(singleVertex_->id())->toKeyVertex();

    for(int i=0; i<loops_.size(); ++i)
        loops_[i].remapPointers(newVAC);

    for(int i=0; i<paths_.size(); ++i)
        paths_[i].remapPointers(newVAC);
}

void CycleHelper::convertTempIdsToPointers(VAC * vac)
{
    // Single vertex
    Cell * cell = vac->getCell(tempId_);
    if(cell)
        singleVertex_ = cell->toKeyVertex();
    else
        singleVertex_= 0;

    // Internal holes
    for(int i=0; i<loops_.size(); ++i)
        loops_[i].convertTempIdsToPointers(vac);

    // External cracks
    for(int i=0; i<paths_.size(); ++i)
        paths_[i].convertTempIdsToPointers(vac);
}

// Replace pointed edges
void CycleHelper::replaceEdges(KeyEdge * oldEdge, const KeyEdgeList & newEdges)
{
    for(int i=0; i<loops_.size(); ++i)
        loops_[i].replaceEdges(oldEdge, newEdges);

    for(int i=0; i<paths_.size(); ++i)
        paths_[i].replaceEdges(oldEdge, newEdges);
}


} // end namespace VectorAnimationComplex

QTextStream & operator<<(QTextStream & out, const VectorAnimationComplex::CycleHelper & hole)
{
    out << Save::openCurlyBrackets();
    out << Save::newField("SingleVertex") << ( hole.singleVertex_ ? hole.singleVertex_->id() : -1 );
    out << Save::newField("Loops") << hole.loops_;
    out << Save::newField("Paths") << hole.paths_;
    out << Save::closeCurlyBrackets();

    return out;
}

QTextStream & operator>>(QTextStream & in, VectorAnimationComplex::CycleHelper & hole)
{
    QString bracket;
    in >> bracket;

    Field field;
    in >> field >> hole.tempId_;
    in >> field >> hole.loops_;
    in >> field >> hole.paths_;

    in >> bracket;

    return in;
}
