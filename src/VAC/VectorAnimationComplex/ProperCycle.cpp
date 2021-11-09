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

#include "ProperCycle.h"
#include "KeyEdge.h"

#include "../SaveAndLoad.h"

#include <QMessageBox>

namespace VectorAnimationComplex
{

ProperCycle::ProperCycle()
{
    // invalid by default, nothing to do (since edges_ is empty)
}

ProperCycle::ProperCycle(const KeyEdgeSet & edgeSetConst)
{
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

    // insert first edge
    halfedges_ << KeyHalfedge(first, true);
    edgeSet.erase(edgeSet.begin());

    // check case where it's a pure loop
    if(first->isClosed())
    {
        if(!edgeSet.isEmpty())
        {
            //QMessageBox::information(0, QObject::tr("operation aborted"),
            //                         QObject::tr("more than one edge and one of them is a pure loop"));
            halfedges_.clear();
            return;

        }
        // else: good!
    }
    else
    {
        // not a pure loop, let's find the chain
        while(!edgeSet.isEmpty())
        {
            KeyHalfedge lastAddedHalfedge = halfedges_.last(); // we know it's not a loop, otherwise couldn't be here
            KeyVertex * lastVertex = lastAddedHalfedge.endVertex();  // hence this is a valid vertex

            // find next
            KeyHalfedge nextHalfedge;
            auto it = edgeSet.begin();
            auto itEnd = edgeSet.end();
            for(;it!=itEnd;++it)
            {
                if((*it)->startVertex() == lastVertex)
                {
                    nextHalfedge = KeyHalfedge(*it, true);
                    break;
                }
                else if((*it)->endVertex() == lastVertex)
                {
                    nextHalfedge = KeyHalfedge(*it, false);
                    break;
                }
            }

            // if found: great, insert it!
            if(nextHalfedge.isValid())
            {
                halfedges_ << nextHalfedge;
                edgeSet.erase(it);
            }
            else
            {
                //QMessageBox::information(0, QObject::tr("operation aborted"),
                //                         QObject::tr("not a valid loop: no valid next edge found"));
                halfedges_.clear();
                return;
            }
        }

        // So far, we've inserted all N edges, and every edge i in [0,N-2]
        // satisfies edges_[i]->endVertex() == edges_[i+1]->startVertex()

        // Check that it's looping
        if(halfedges_.last().endVertex() != halfedges_.first().startVertex())
        {
            //QMessageBox::information(0, QObject::tr("operation aborted"),
            //                         QObject::tr("not a valid loop: last edge not compatible with first one"));
            halfedges_.clear();
            return;
        }

        // Check that it's simple
        KeyVertexSet vertices;
        for(KeyHalfedge he: halfedges_)
        {
            KeyVertex * vertex = he.startVertex();
            if(vertices.contains(vertex))
            {
                //QMessageBox::information(0, QObject::tr("operation aborted"),
                //                         QObject::tr("not a valid loop: not simple"));
                halfedges_.clear();
                return;
            }
            else
            {
                vertices << vertex;
            }
        }

        // Done :-) If you're here you have a valid simple loop
    }

}

bool ProperCycle::isValid() const
{
    return !halfedges_.isEmpty();
}

Time ProperCycle::time() const
{
    return halfedges_[0].time();
}

int  ProperCycle::size() const
{
    return halfedges_.size();
}
KeyHalfedge ProperCycle::operator[](int i) const
{
    return halfedges_[i];
}

void ProperCycle::remapPointers(VAC * newVAC)
{
    for(int i=0; i<halfedges_.size(); ++i)
        halfedges_[i].remapPointers(newVAC);
}

void ProperCycle::convertTempIdsToPointers(VAC * vac)
{
    for(int i=0; i<halfedges_.size(); ++i)
        halfedges_[i].convertTempIdsToPointers(vac);
}

// Replace pointed edges
void ProperCycle::replaceEdges(KeyEdge * oldEdge, const KeyEdgeList & newEdges)
{
    QList<KeyHalfedge> newHalfedges;

    for(KeyHalfedge he: halfedges_)
    {
        if(he.edge == oldEdge)
        {
            // Replace halfedge
            if(he.side)
            {
                for(int i=0; i<newEdges.size(); ++i)
                    newHalfedges << KeyHalfedge(newEdges[i], he.side);
            }
            else
            {
                for(int i=newEdges.size()-1; i>=0; --i)
                    newHalfedges << KeyHalfedge(newEdges[i], he.side);
            }
        }
        else
        {
            // Keep halfedge as is
            newHalfedges << he;
        }
    }

    halfedges_ = newHalfedges;
}

} // end namespace VectorAnimationComplex

QTextStream & operator<<(QTextStream & out, const VectorAnimationComplex::ProperCycle & loop)
{
    out << loop.halfedges_;

    return out;
}

QTextStream & operator>>(QTextStream & in, VectorAnimationComplex::ProperCycle & loop)
{
    in >> loop.halfedges_;

    return in;
}
