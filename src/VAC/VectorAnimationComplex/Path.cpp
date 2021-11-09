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

#include "Path.h"
#include "KeyVertex.h"
#include "KeyEdge.h"
#include "EdgeGeometry.h"
#include "VAC.h"
#include "../SaveAndLoad.h"

#include <QMessageBox>

namespace VectorAnimationComplex
{

Path::PathType Path::type() const
{
    if(vertex_)
    {
        return SingleVertex;
    }
    else if (halfedges_.isEmpty())
    {
        return Invalid;
    }
    else
    {
        return OpenHalfedgeList;
    }
}

bool Path::isValid() const
{
    return type() != Invalid;
}


Path::Path() :
    tempId_(-1),
    vertex_(0)
{
    // invalid by default, nothing to do (since edges_ is empty)
}

Path::Path(KeyVertex * instantVertex) :
    tempId_(-1),
    vertex_(instantVertex)
{
}

Path::Path(const QList<KeyHalfedge> halfedges) :
    tempId_(-1),
    vertex_(0),
    halfedges_(halfedges)
{
    // Check that it's valid
    if(halfedges_.size() == 0)
        return;
    for(int i=0; i<halfedges_.size()-1; i++)
    {
        if( (halfedges_[i].isClosed()) || (halfedges_[i].endVertex() != halfedges_[i+1].startVertex()) )
        {
            halfedges_.clear();
            break;
        }
    }
}

Path::Path(const KeyEdgeSet & edgeSetConst) :
    tempId_(-1),
    vertex_(0)
{
    // TODO: For now, assume it is a strict path

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
        // What's commented below was copy-pasted from Cycle.cpp. For Path.cpp, closed edges make no sense at all
        //if(!edgeSet.isEmpty())
        //{
        //    //QMessageBox::information(0, QObject::tr("operation aborted"),
            //                         QObject::tr("more than one edge and one of them is a pure loop"));
            halfedges_.clear();
            return;

        //}
        // else: good!
    }
    else
    {
        // not a pure loop, let's find the chain
        while(!edgeSet.isEmpty())
        {
            KeyVertex * lastVertex = halfedges_.last().endVertex();  // hence this is a valid vertex
            KeyVertex * firstVertex = halfedges_.first().startVertex();  // hence this is a valid vertex

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

            // if found: great, append it!
            if(nextHalfedge.isValid())
            {
                halfedges_ << nextHalfedge;
                edgeSet.erase(it);
            }
            else // can still try to prepend one
            {
                // find next
                KeyHalfedge previousHalfedge;
                it = edgeSet.begin();
                for(;it!=itEnd;++it)
                {
                    if((*it)->endVertex() == firstVertex)
                    {
                        previousHalfedge = KeyHalfedge(*it, true);
                        break;
                    }
                    else if((*it)->startVertex() == firstVertex)
                    {
                        previousHalfedge = KeyHalfedge(*it, false);
                        break;
                    }
                }

                // if found: great, prepend it!
                if(previousHalfedge.isValid())
                {
                    halfedges_.prepend(nextHalfedge);
                    edgeSet.erase(it);
                }
                else
                {
                    halfedges_.clear();
                    return;
                }
            }
        }

        // So far, we've inserted all N edges, and every edge i in [0,N-2]
        // satisfies edges_[i]->endVertex() == edges_[i+1]->startVertex()

        // Check that it's looping
        //if(halfedges_.last().endVertex() != halfedges_.first().startVertex())
        //{
            //QMessageBox::information(0, QObject::tr("operation aborted"),
            //                         QObject::tr("not a valid loop: last edge not compatible with first one"));
        //    halfedges_.clear();
        //    return;
        //}

        // Check that it's simple
        //KeyVertexSet vertices;
        //for(KeyHalfEdge he: halfedges_)
        //{
        //    KeyVertex * vertex = he.startVertex();
        //    if(vertices.contains(vertex))
        //    {
                //QMessageBox::information(0, QObject::tr("operation aborted"),
                //                         QObject::tr("not a valid loop: not simple"));
        //        halfedges_.clear();
        //        return;
        //    }
        //    else
        //    {
        //        vertices << vertex;
        //    }
        //}

        // Done :-) If you're here you have a valid simple loop
    }
}

// Conversion from proper path
Path::Path(const ProperPath & properPath) :
    tempId_(-1),
    vertex_(0)
{
    if(properPath.isValid())
    {
        for(int i=0; i<properPath.size(); ++i)
        {
            halfedges_ << properPath[i];
        }
    }
}
// Conversion from proper path
Path::Path(const ProperCycle & properCycle) :
    tempId_(-1),
    vertex_(0)
{
    if(properCycle.isValid() && !properCycle[0].isClosed())
    {
        for(int i=0; i<properCycle.size(); ++i)
        {
            halfedges_ << properCycle[i];
        }
    }
}

Time Path::time() const
{
    assert(isValid());
    switch(type())
    {
    case SingleVertex:
        return vertex_->time();
        break;
    case OpenHalfedgeList:
        return halfedges_[0].time();
        break;
    case Invalid:
        return Time();
        break;
    }

    return Time();
}

KeyVertex * Path::singleVertex() const
{
    return vertex_;
}

int  Path::size() const
{
    return halfedges_.size();
}

KeyHalfedge Path::operator[](int i) const
{
    return halfedges_[i];
}

// The set of cells this helper points to
KeyCellSet Path::cells() const
{
    KeyCellSet res;


    switch(type())
    {
    case SingleVertex:
        res << vertex_;
        break;

    case OpenHalfedgeList:
        for(int i=0; i<size(); ++i)
        {
            KeyHalfedge he = halfedges_[i];
            res << he.startVertex() << he.edge;
        }
        res << halfedges_.last().endVertex();
        break;

    case Invalid:
        break;
    }

    return res;
}

KeyVertex * Path::startVertex() const
{
    assert(isValid());

    if(type() == SingleVertex)
    {
        return singleVertex();
    }
    else
    {
        return halfedges_.first().startVertex();
    }
}

KeyVertex * Path::endVertex() const
{
    assert(isValid());

    if(type() == SingleVertex)
    {
        return singleVertex();
    }
    else
    {
        return halfedges_.last().endVertex();
    }
}



void Path::remapPointers(VAC * newVAC)
{
    for(int i=0; i<halfedges_.size(); ++i)
        halfedges_[i].remapPointers(newVAC);

    if(vertex_)
    {
        Cell * c = newVAC->getCell(vertex_->id());
        vertex_ = c->toKeyVertex();
    }
}

void Path::convertTempIdsToPointers(VAC * vac)
{
    // Vertex
    Cell * cell = vac->getCell(tempId_);
    if(cell)
        vertex_ = cell->toKeyVertex();
    else
        vertex_ = 0;

    // Halfedges
    for(int i=0; i<halfedges_.size(); ++i)
        halfedges_[i].convertTempIdsToPointers(vac);
}

// The XML file syntax for paths is either:
//    [e1,beta1 e2,beta2 ... eN,betaN]
// or
//    [v]

QString Path::toString() const
{
    QString res;

    if(vertex_)
    {
        // Vertex
        res += "[" + QString().setNum(vertex_->id()) + "]";
    }
    else
    {
        // Halfedges
        res += "[";
        for(int i=0; i<halfedges_.size(); ++i)
        {
            if(i>0)
                res += " ";
            res += QString().setNum(halfedges_[i].edge->id()) + (halfedges_[i].side ? "+" : "-");
        }
        res += "]";
    }

    return res;
}

void Path::fromString(const QString & str)
{
    // Clear
    tempId_ = -1;
    vertex_ = 0;
    halfedges_.clear();

    // Split at ',', '[', ']', or any whitespace character
    QStringList strList = str.split(QRegExp("[\\,\\s\\[\\]]"), QString::SkipEmptyParts);

    // Get some info to determine cycle type
    QString firstStr = strList[0];
    QChar c =  firstStr.at(firstStr.length()-1);

    // Switch depending on type
    if(strList.size() == 1 && c != '+' && c != '-')
    {
        // Vertex
        tempId_ = strList[0].toInt();
    }
    else
    {
        // Halfedges
        int n = strList.size();
        for(int i=0; i<n; ++i)
        {
            QString str = strList[i];
            int l = str.length();
            QChar side = str.at(l-1);
            QString edge = str.left(l-1);

            KeyHalfedge h;
            h.tempId_ = edge.toInt();
            h.side = (side == '+');
            halfedges_ << h;
        }
    }
}


// Replace pointed edges
void Path::replaceEdges(KeyEdge * oldEdge, const KeyEdgeList & newEdges)
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

void Path::replaceVertex(KeyVertex * oldVertex, KeyVertex * newVertex)
{
    if(vertex_ == oldVertex)
        vertex_ = newVertex;
}


void Path::replaceHalfedge(const KeyHalfedge & oldHalfedge, const KeyHalfedge & newHalfedge)
{
    for(int j=0; j<halfedges_.size(); j++)
    {
        if(halfedges_[j].edge == oldHalfedge.edge)
        {
            halfedges_[j].edge = newHalfedge.edge;
            halfedges_[j].side = ((halfedges_[j].side == oldHalfedge.side) == newHalfedge.side);
        }
    }
}

double Path::length() const
{
    assert(isValid());

    if(type() == SingleVertex)
    {
        return 0;
    }
    else
    {
        double res = 0;
        for(KeyHalfedge he: halfedges_)
            res += he.edge->geometry()->length();
        return res;
    }
}

void Path::sample(int numSamples, QList<EdgeSample> & out) const
{
    assert(isValid());
    out.clear();

    if(type() == SingleVertex)
    {
        Eigen::Vector2d pos = singleVertex()->pos();
        EdgeSample posSample(pos[0],pos[1],0);
        for(int i=0; i<numSamples; ++i)
            out << posSample;
    }
    else
    {
        assert(numSamples >= 2);
        double ds = length()/(numSamples-1);

        double cumulativeLength = 0;
        int indexHe = 0;
        KeyHalfedge he = halfedges_[indexHe];
        for(int i=0; i<numSamples; ++i)
        {
            double s = i*ds;
            while ( (s > cumulativeLength + he.length()) && (indexHe+1 < halfedges_.size()) )
            {
                cumulativeLength += he.length();
                he = halfedges_[++indexHe];
            }
            out << he.sample(s-cumulativeLength);
        }
    }

}

void Path::sample(int numSamples, QList<Eigen::Vector2d> & out) const
{
    assert(isValid());
    out.clear();

    if(type() == SingleVertex)
    {
        Eigen::Vector2d pos = singleVertex()->pos();
        for(int i=0; i<numSamples; ++i)
            out << pos;
    }
    else
    {
        assert(numSamples >= 2);
        double ds = length()/(numSamples-1);

        double cumulativeLength = 0;
        int indexHe = 0;
        KeyHalfedge he = halfedges_[indexHe];
        for(int i=0; i<numSamples; ++i)
        {
            double s = i*ds;
            while ( (s > cumulativeLength + he.length()) && (indexHe+1 < halfedges_.size()) )
            {
                cumulativeLength += he.length();
                he = halfedges_[++indexHe];
            }
            out << he.pos(s-cumulativeLength);
        }
    }
}

Path Path::reversed() const
{
    Path res;
    res.vertex_ = vertex_;

    int n=size();
    for(int i=0; i<n; ++i)
    {
        KeyHalfedge h = halfedges_[n-1-i];
        h.side = !h.side;
        res.halfedges_ << h;
    }

    return res;
}

} // end namespace VectorAnimationComplex

QTextStream & operator<<(QTextStream & out, const VectorAnimationComplex::Path & Path)
{
    // Vertex
    if(Path.vertex_)
        out << Path.vertex_->id();
    else
        out << -1;

    // Halfedges
    out << " " << Path.halfedges_;

    return out;
}

QTextStream & operator>>(QTextStream & in, VectorAnimationComplex::Path & Path)
{
    in >> Path.tempId_ >> Path.halfedges_;

    return in;
}
