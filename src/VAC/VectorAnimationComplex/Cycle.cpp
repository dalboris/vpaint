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

#include "Cycle.h"
#include "KeyVertex.h"
#include "KeyEdge.h"
#include "EdgeGeometry.h"
#include "VAC.h"

#include "../SaveAndLoad.h"

#include <QMessageBox>

namespace VectorAnimationComplex
{

Cycle::CycleType Cycle::type() const
{
    if(vertex_)
    {
        return SingleVertex;
    }
    else if (halfedges_.isEmpty())
    {
        return Invalid;
    }
    else if (halfedges_[0].isClosed())
    {
        return ClosedHalfedge;
    }
    else
    {
        return OpenHalfedgeList;
    }
}

bool Cycle::isValid() const
{
    return type() != Invalid;
}


Cycle::Cycle() :
    tempId_(-1),
    vertex_(0),
    s0_(0)
{
    // invalid by default, nothing to do (since edges_ is empty)
}

Cycle::Cycle(KeyVertex * instantVertex) :
    tempId_(-1),
    vertex_(instantVertex),
    s0_(0)
{
}

Cycle::Cycle(const QList<KeyHalfedge> & halfedges) :
    tempId_(-1),
    vertex_(0),
    s0_(0),
    halfedges_(halfedges)
{
    // Check that it's valid
    if(halfedges_.size() == 0)
        return;
    if(halfedges_.first().startVertex() != halfedges_.last().endVertex())
    {
        halfedges_.clear();
        return;
    }
    for(int i=0; i<halfedges_.size()-1; i++)
    {
        if(halfedges_[i].endVertex() != halfedges_[i+1].startVertex())
        {
            halfedges_.clear();
            break;
        }
    }
    // TODO: Check for validity of simple cycles
}

Cycle::Cycle(const KeyEdgeSet & edgeSetConst) :
    tempId_(-1),
    vertex_(0),
    s0_(0)
{
    // TODO: For now, assume it is a simple loop

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
        for(KeyHalfedge he: qAsConst(halfedges_))
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

Cycle::Cycle(const ProperCycle & properCycle) :
    tempId_(-1),
    vertex_(0),
    s0_(0)
{
    if(properCycle.isValid())
    {
        for(int i=0; i<properCycle.size(); ++i)
        {
            halfedges_ << properCycle[i];
        }
    }
}

Time Cycle::time() const
{
    switch(type())
    {
    case SingleVertex:
        return vertex_->time();
        break;
    case ClosedHalfedge:
    case OpenHalfedgeList:
        return halfedges_[0].time();
        break;
    case Invalid:
        return Time();
        break;
    }

    return Time();
}

KeyVertex * Cycle::singleVertex() const
{
    return vertex_;
}

KeyHalfedge Cycle::closedHalfedge() const
{
    return halfedges_[0];
}

int  Cycle::size() const
{
    return halfedges_.size();
}

KeyHalfedge Cycle::operator[](int i) const
{
    return halfedges_[i];
}

// The set of cells this helper points to
KeyCellSet Cycle::cells() const
{
    KeyCellSet res;


    switch(type())
    {
    case SingleVertex:
        res << vertex_;
        break;

    case ClosedHalfedge:
        res << halfedges_[0].edge;
        break;

    case OpenHalfedgeList:
        for(int i=0; i<size(); ++i)
        {
            KeyHalfedge he = halfedges_[i];
            res << he.startVertex() << he.edge;
        }
        break;

    case Invalid:
        break;
    }

    return res;
}


void Cycle::remapPointers(VAC * newVAC)
{
    for(int i=0; i<halfedges_.size(); ++i)
        halfedges_[i].remapPointers(newVAC);

    if(vertex_)
    {
        Cell * c = newVAC->getCell(vertex_->id());
        vertex_ = c->toKeyVertex();
    }
}

void Cycle::convertTempIdsToPointers(VAC * vac)
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

// Replace pointed edges
void Cycle::replaceEdges(KeyEdge * oldEdge, const KeyEdgeList & newEdges)
{
    QList<KeyHalfedge> newHalfedges;

    for(KeyHalfedge he: qAsConst(halfedges_))
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

void Cycle::replaceVertex(KeyVertex * oldVertex, KeyVertex * newVertex)
{
    if(vertex_ == oldVertex)
        vertex_ = newVertex;
}

void Cycle::replaceHalfedge(const KeyHalfedge & oldHalfedge, const KeyHalfedge & newHalfedge)
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

double Cycle::length() const
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

void Cycle::sample(int numSamples, QList<EdgeSample> & out) const
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
        QList<EdgeSample> outAux;

        assert(numSamples >= 2);
        double l = length();
        double ds = l/(numSamples-1);

        // Computing unoffset cycle
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
            outAux << he.sample(s-cumulativeLength);
        }

        // Apply offset
        int i0 = std::floor( numSamples * s0_ + 0.5);
        if(i0 < 0)
            i0 = 0;
        if(i0 > numSamples - 1)
            i0 = numSamples - 1;
        for(int i=i0; i<numSamples; ++i)
            out << outAux[i];
        for(int i=0; i<i0; ++i)
            out << outAux[i];
    }

}

void Cycle::sample(QList<Eigen::Vector2d> & out) const
{
    double ds = 3.0;
    double numSamples = length()/ds + 4;
    sample(numSamples,out);
}

void Cycle::sample(int numSamples, QList<Eigen::Vector2d> & out) const
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
        QList<Eigen::Vector2d> outAux;

        // Computing unoffset cycle
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
            outAux << he.pos(s-cumulativeLength);
        }

        // Apply offset
        int i0 = std::floor( numSamples * s0_ + 0.5);
        if(i0 < 0)
            i0 = 0;
        if(i0 > numSamples - 1)
            i0 = numSamples - 1;
        for(int i=i0; i<numSamples; ++i)
            out << outAux[i];
        for(int i=0; i<i0; ++i)
            out << outAux[i];
    }
}

double Cycle::totalCurvature() const
{
    CycleType t = type();

    if(t == Invalid || t == SingleVertex)
    {
        return 0;
    }
    else
    {
        // Compute sampling
        QList<Eigen::Vector2d> samples;
        sample(samples);

        // Compute total curvature
        int n = samples.size();
        if(n <= 4)
            return 0;
        double res = 0;
        for(int i=0; i<n-1; ++i) // note, last sample ignored, since first == last
        {
            // Find three points involved
            int ia = (n-1+i-1) % (n-1);
            int ib = (n-1+i)   % (n-1);
            int ic = (n-1+i+1) % (n-1);
            Eigen::Vector2d a = samples[ia];
            Eigen::Vector2d b = samples[ib];
            Eigen::Vector2d c = samples[ic];

            // Compute diff of angle between AB and BC
            Eigen::Vector2d ab = b-a;
            Eigen::Vector2d bc = c-b;
            double dot = ab[0]*bc[0] + ab[1]*bc[1];
            double det = ab[0]*bc[1] - ab[1]*bc[0];
            double angle = std::atan2(det,dot);

            // Accumulate curvature
            res += angle;
        }
        return res;
    }
}

int Cycle::turningNumber() const
{
    // Note: std::round is C++11, not supported by Visual Studio 2012

    // return (int) std::round(0.5 * totalCurvature() / M_PI);
    return (int) std::floor(0.5 + 0.5 * totalCurvature() / M_PI);
}

double Cycle::s0() const
{
    return s0_;
}

void Cycle::setStartingPoint(double s0)
{
    s0_ = s0;
}


Cycle Cycle::reversed() const
{
    Cycle res;
    res.vertex_ = vertex_;

    int n=size();
    for(int i=0; i<n; ++i)
    {
        KeyHalfedge h = halfedges_[n-1-i];
        h.side = !h.side;
        res.halfedges_ << h;
    }

    if(s0_ != 0)
    {
        res.s0_ = 1 - s0_;
    }
    else
    {
        res.s0_ = 0;
    }

    return res;
}




// The XML file syntax for cycles is either:
//    [e1,beta1 e2,beta2 ... eN,betaN]
// or
//    [v]
//
// s0 is not specified part of this syntax, and initialized to zero.
// If a non-zero offset is needed, do it manually after.

QString Cycle::toString() const
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

void Cycle::fromString(const QString & str)
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

} // end namespace VectorAnimationComplex





// The deprecated file syntax for cycles is either:
//    vertexID s0 [ (e1,beta1) , (e2,beta2) , ... , (eN,betaN) ]
// or
//    vertexID [ (e1,beta1) , (e2,beta2) , ... , (eN,betaN) ]
//
// when s0 is omitted, it is implicitely equal to zero

QTextStream & operator<<(QTextStream & out, const VectorAnimationComplex::Cycle & cycle)
{
    // Vertex
    if(cycle.vertex_)
        out << cycle.vertex_->id();
    else
        out << -1;

    // Starting point
    if(cycle.s0_ != 0)
        out << " " << cycle.s0_;

    // Halfedges
    out << " " << cycle.halfedges_;

    return out;
}

#include <QtDebug>

QTextStream & operator>>(QTextStream & in, VectorAnimationComplex::Cycle & cycle)
{
    // Read cycle input as string
    QString stringCycle;
    QChar c;
    do
    {
        in >> c;
        stringCycle += c;
    }
    while (c != ']');

    // Separate the halfedge part to the beginning
    int i = stringCycle.indexOf('[');
    QString stringVertexStartingPoint = stringCycle.left(i);
    QString stringHalfedges = stringCycle.right(stringCycle.size()-i);
    QStringList stringListVertexStartingPoint = stringVertexStartingPoint.split(' ',QString::SkipEmptyParts);

    // Process vertex and starting point
    if(stringListVertexStartingPoint.size() >= 1)
        cycle.tempId_ = stringListVertexStartingPoint[0].toInt();
    else
        cycle.tempId_ = -1;

    if(stringListVertexStartingPoint.size() >= 2)
        cycle.s0_ = stringListVertexStartingPoint[1].toDouble();
    else
        cycle.s0_ = 0;

    // Process halfedges
    QTextStream inHalfedges(&stringHalfedges);
    inHalfedges >> cycle.halfedges_;

    // Return
    return in;
}
