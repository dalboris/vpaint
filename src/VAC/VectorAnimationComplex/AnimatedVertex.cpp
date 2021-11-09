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

#include "AnimatedVertex.h"

#include "KeyVertex.h"
#include "InbetweenVertex.h"
#include "VAC.h"

#include <assert.h>

namespace VectorAnimationComplex
{

AnimatedVertex::AnimatedVertex()
{
}

AnimatedVertex::AnimatedVertex(const InbetweenVertexList & inbetweenVertices) :
    inbetweenVertices_(inbetweenVertices)
{
    int n = inbetweenVertices_.size();
    assert(n >= 1);
    for(int i=0; i<n-1; ++i)
    {
        assert(inbetweenVertices_[i]->afterVertex() == inbetweenVertices_[i+1]->beforeVertex());
    }
}

bool AnimatedVertex::isValid() const
{
    return !inbetweenVertices_.isEmpty();
}

InbetweenVertexList AnimatedVertex::inbetweenVertices() const
{
    return inbetweenVertices_;
}

KeyVertex * AnimatedVertex::beforeVertex() const
{
    return inbetweenVertices_.first()->beforeVertex();
}

KeyVertex * AnimatedVertex::afterVertex() const
{
    return inbetweenVertices_.last()->afterVertex();
}

VertexCellSet AnimatedVertex::vertices() const
{
    VertexCellSet res;
    int n = inbetweenVertices_.size();
    for(int i=0; i<n-1; ++i)
    {
        res << inbetweenVertices_[i];
        res << inbetweenVertices_[i]->afterVertex();
    }
    res << inbetweenVertices_.last();
    return res;
}

void AnimatedVertex::replaceCells(InbetweenVertex * old, InbetweenVertex * new1, InbetweenVertex * new2)
{
    assert(old->beforeVertex() == new1->beforeVertex());
    assert(new1->afterVertex() == new2->beforeVertex());
    assert(new2->afterVertex() == old->afterVertex());

    InbetweenVertexList newVertices;
    int n = inbetweenVertices_.size();
    for(int i=0; i<n; ++i)
    {
        if(inbetweenVertices_[i] == old)
        {
            newVertices << new1;
            newVertices << new2;
        }
        else
        {
            newVertices << inbetweenVertices_[i];
        }
    }

    inbetweenVertices_ = newVertices;
}

// Replace pointed vertex
void AnimatedVertex::replaceVertex(KeyVertex * , KeyVertex * )
{
    // Nothing to do
}

int AnimatedVertex::size() const
{
    return inbetweenVertices_.size();
}

InbetweenVertex * AnimatedVertex::operator[] (int i) const
{
    return inbetweenVertices_[i];
}

Eigen::Vector2d AnimatedVertex::pos(Time time) const
{
    VertexCellSet set = vertices();
    set << beforeVertex() << afterVertex();
    for(VertexCell * v: set)
    {
        if(v->exists(time))
            return v->pos(time);
    }
    assert(false && "no vertices at requested time");
    return Eigen::Vector2d(0,0);
}

void AnimatedVertex::remapPointers(VAC * newVAC)
{
    for(int i=0; i<inbetweenVertices_.size(); ++i)
    {
        inbetweenVertices_[i] = newVAC->getCell(inbetweenVertices_[i]->id())->toInbetweenVertex();
    }
}

void AnimatedVertex::convertTempIdsToPointers(VAC * vac)
{
    for(int i=0; i<tempIds_.size(); ++i)
    {
        inbetweenVertices_ << vac->getCell(tempIds_[i])->toInbetweenVertex();
    }
    tempIds_.clear();
}


QString AnimatedVertex::toString() const
{
    QString res;

    res += "[";
    for(int i=0; i<inbetweenVertices_.size(); ++i)
    {
        if(i>0)
            res += " ";
        res += QString().setNum(inbetweenVertices_[i]->id());
    }
    res += "]";

    return res;
}

void AnimatedVertex::fromString(const QString & str)
{
    // Clear
    tempIds_.clear();

    // Split at ',', '[', ']', or any whitespace character
    QStringList strList = str.split(QRegExp("[\\,\\s\\[\\]]"), QString::SkipEmptyParts);

    int n = strList.size();
    for(int i=0; i<n; ++i)
        tempIds_ << strList[i].toInt();
}


} // end namespace VectorAnimationComplex

QTextStream & operator<<(QTextStream & out, const VectorAnimationComplex::AnimatedVertex & animatedVertex)
{
    out << "[";
    for(int i=0; i<animatedVertex.inbetweenVertices_.size(); ++i)
    {
        if(i!=0) out << " ,";
        out << " " << animatedVertex.inbetweenVertices_[i]->id();
    }
    out << " ]";


    return out;
}

QTextStream & operator>>(QTextStream & in, VectorAnimationComplex::AnimatedVertex & animatedVertex)
{
    // put the list to read as a string
    QString listAsString;
    in >> listAsString; // read "["
    int openedBracket = 1;
    char c;
    while(openedBracket != 0)
    {
        in >> c;
        if(c == '[')
            openedBracket++;
        if(c == ']')
            openedBracket--;

        listAsString.append(c);
    }

    // test if the list is void
    QString copyString = listAsString;
    QTextStream test(&copyString);
    QString b1, b2;
    test >> b1 >> b2;

    // if not void
    if(b2 != "]")
    {
        QTextStream newIn(&listAsString);
        newIn >> b1; // read "["
        QString delimiter(",");
        while(delimiter == ",")
        {
            int id;
            newIn >> id;
            animatedVertex.tempIds_ << id;
            newIn >> delimiter;
        }
    }

    return in;
}
