// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "ProperPath.h"
#include "KeyEdge.h"

#include "../SaveAndLoad.h"

#include <QMessageBox>

namespace VectorAnimationComplex
{

ProperPath::ProperPath()
{
    // invalid by default, nothing to do (since edges_ is empty)
}

ProperPath::ProperPath(const KeyEdgeSet & edgeSetConst)
{
    // If no edge, then invalid
    if(edgeSetConst.isEmpty())
        return;

    // if not all edges at same time, then invalid
    KeyEdge * first = *edgeSetConst.begin();
    Time t = first->time();
    foreach(KeyEdge * iedge, edgeSetConst)
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
    if(first->isClosed())
    {
        //QMessageBox::information(0, QObject::tr("operation aborted"),
        //                         QObject::tr("one of the edge is a pure loop"));
        halfedges_.clear();
        return;

    }

    // not a pure loop, let's find the chain
    while(!edgeSet.isEmpty())
    {
        KeyHalfedge lastHalfedge = halfedges_.last(); // we know it's not a loop, otherwise couldn't be here
        KeyVertex * lastVertex = lastHalfedge.endVertex();  // hence this is a valid vertex

        KeyHalfedge firstHalfedge = halfedges_.first(); // we know it's not a loop, otherwise couldn't be here
        KeyVertex * firstVertex = firstHalfedge.startVertex();  // hence this is a valid vertex

        // find next
        auto it = edgeSet.begin();
        auto itEnd = edgeSet.end();
        for(;it!=itEnd;++it)
        {
            if((*it)->startVertex() == lastVertex)
            {
                halfedges_.append(KeyHalfedge(*it, true));
                break;
            }
            else if((*it)->endVertex() == lastVertex)
            {
                halfedges_.append(KeyHalfedge(*it, false));
                break;
            }
            else if((*it)->endVertex() == firstVertex)
            {
                halfedges_.prepend(KeyHalfedge(*it, true));
                break;
            }
            else if((*it)->startVertex() == firstVertex)
            {
                halfedges_.prepend(KeyHalfedge(*it, false));
                break;
            }
        }

        // if found: great!
        if(it!=itEnd)
        {
            edgeSet.erase(it);
        }
        else
        {
            //QMessageBox::information(0, QObject::tr("operation aborted"),
            //                         QObject::tr("not a valid path: no valid next edge found"));
            halfedges_.clear();
            return;
        }
    }

    // So far, we've inserted all N edges, and every edge i in [0,N-2]
    // satisfies edges_[i]->endVertex() == edges_[i+1]->startVertex()

    // Check that it's not looping
    if(halfedges_.last().endVertex() == halfedges_.first().startVertex())
    {
        //QMessageBox::information(0, QObject::tr("operation aborted"),
        //                         QObject::tr("the path is looping, this is not allowed"));
        halfedges_.clear();
        return;
    }

    // Check that it's simple
    KeyVertexSet vertices;
    foreach(KeyHalfedge he, halfedges_)
    {
        KeyVertex * vertex = he.startVertex();
        if(vertices.contains(vertex))
        {
            //QMessageBox::information(0, QObject::tr("operation aborted"),
            //                         QObject::tr("not a valid path: not simple"));
            halfedges_.clear();
            return;
        }
        else
        {
            vertices << vertex;
        }
    }

    // Done :-) If you're here you have a valid simple path


}

bool ProperPath::isValid() const
{
    return !halfedges_.isEmpty();
}

Time ProperPath::time() const
{
    return halfedges_[0].time();
}

int  ProperPath::size() const
{
    return halfedges_.size();
}
KeyHalfedge ProperPath::operator[](int i) const
{
    return halfedges_[i];
}

void ProperPath::remapPointers(VAC * newVAC)
{
    for(int i=0; i<halfedges_.size(); ++i)
        halfedges_[i].remapPointers(newVAC);
}
void ProperPath::save(QTextStream & out)
{
    out << "[ ";
    foreach(KeyHalfedge he, halfedges_)
    {
        he.save(out);
        out << " ";
    }
    out << "]";
}

void ProperPath::convertTempIdsToPointers(VAC * vac)
{
    for(int i=0; i<halfedges_.size(); ++i)
        halfedges_[i].convertTempIdsToPointers(vac);
}

// Replace pointed edges
void ProperPath::replaceEdges(KeyEdge * oldEdge, const KeyEdgeList & newEdges)
{
    QList<KeyHalfedge> newHalfedges;

    foreach(KeyHalfedge he, halfedges_)
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


QTextStream & operator<<(QTextStream & out, const VectorAnimationComplex::ProperPath & path)
{
    out << path.halfedges_;

    return out;
}

QTextStream & operator>>(QTextStream & in, VectorAnimationComplex::ProperPath & path)
{
    in >> path.halfedges_;

    return in;
}
