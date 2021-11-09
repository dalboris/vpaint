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

#include "KeyVertex.h"
#include "KeyEdge.h"
#include "InbetweenVertex.h"
#include "EdgeGeometry.h"

#include "../OpenGL.h"
#include <QtDebug>
#include <QTextStream>
#include "../SaveAndLoad.h"
#include "../DevSettings.h"
#include "../Global.h"

#include "../XmlStreamReader.h"
#include "../XmlStreamWriter.h"

namespace VectorAnimationComplex
{

KeyVertex::KeyVertex(VAC * vac, Time time, const Eigen::Vector2d & pos) :
    Cell(vac),
    KeyCell(vac, time),
    VertexCell(vac),
    pos_(pos)
{
    initColor();

    size_ = global()->edgeWidth() * 1.7;
}

KeyVertex::KeyVertex(VAC * vac, Time time, const EdgeSample& sample) :
    Cell(vac),
    KeyCell(vac, time),
    VertexCell(vac),
    pos_(sample.x(), sample.y())
{
    initColor();

    size_ = sample.width() * 1.7;
}

KeyVertex::KeyVertex(VAC * vac, Time time) :
    Cell(vac),
    KeyCell(vac, time),
    VertexCell(vac),
    pos_(0,0)
{
    initColor();

    size_ = global()->edgeWidth() * 1.7;
}

void KeyVertex::initColor()
{
    color_[0] = 0;
    color_[1] = 0;
    color_[2] = 0;
    color_[3] = 1;
}

void KeyVertex::save_(QTextStream & out)
{
    // Base classes
    Cell::save_(out);
    KeyCell::save_(out);
    VertexCell::save_(out);

    // Position
    out << Save::newField("Pos") << "(" << pos_[0] << "," << pos_[1] << ")";

    // Size
    out << Save::newField("Size") << size_;

    // Tangent Edges // DEPRECATED
    out << Save::newField("TangentEdges") << tangentEdges_;
}

void KeyVertex::write_(XmlStreamWriter & xml) const
{
    // Base classes
    Cell::write_(xml);
    KeyCell::write_(xml);
    VertexCell::write_(xml);

    // Position
    xml.writeAttribute("position", QString().setNum(pos_[0]) + " " + QString().setNum(pos_[1]));

    // Size // TODO, must be in style
    //out << Save::newField("Size") << size_;
}

QString KeyVertex::xmlType_() const
{
    return "vertex";
}

KeyVertex::KeyVertex(VAC * vac, XmlStreamReader & xml) :
    Cell(vac, xml),
    KeyCell(vac, xml),
    VertexCell(vac, xml)
{
    initColor();

    // Position
    QString stringPos = xml.attributes().value("position").toString();
    QStringList list = stringPos.split(" ");
    pos_[0] = list[0].toDouble();
    pos_[1] = list[1].toDouble();

    // Size
    //in >> field >> size_;

    // Tangent Edges
    //in >> tangentEdges_;

}

KeyVertex::KeyVertex(VAC * vac, QTextStream & in) :
    Cell(vac, in),
    KeyCell(vac, in),
    VertexCell(vac, in)
{
    initColor();

    // Position
    Field field;
    QString stringPos;
    in >> /*field >>*/ stringPos; // Reason of comment: see Cell::Cell(VAC * vac, QTextStream & in)
    // be  careful,  would not  give
    // the   expexcted   result   if
    // whitespace are in pos
    QStringList list = stringPos.split(QRegExp("\\s*[\\(\\,\\)]\\s*"),
                             QString::SkipEmptyParts);
    pos_[0] = list[0].toDouble();
    pos_[1] = list[1].toDouble();

    // Size
    in >> field >> size_;

    // Tangent Edges
    in >> tangentEdges_;
}

void KeyVertex::read2ndPass()
{
    // Base classes
    Cell::read2ndPass();
    KeyCell::read2ndPass();
    VertexCell::read2ndPass();

    // Tangent Edges
    for(int i=0; i<tangentEdges_.size(); ++i)
    {
        tangentEdges_[i].first.convertTempIdsToPointers(vac());
        tangentEdges_[i].second.convertTempIdsToPointers(vac());
    }
}


KeyVertex * KeyVertex::clone()
{
    return new KeyVertex(this);
}

void KeyVertex::remapPointers(VAC * newVAC)
{
    Cell::remapPointers(newVAC);
    KeyCell::remapPointers(newVAC);
    VertexCell::remapPointers(newVAC);

    // no pointers in this class
}

KeyVertex::KeyVertex(KeyVertex * other):
    Cell(other),
    KeyCell(other),
    VertexCell(other)
{
    initColor();

    pos_ = other->pos_;
    size_ = other->size_;
}

KeyVertex::~KeyVertex()
{
}


void KeyVertex::computePosFromEdges()
{
    // The mean of geometry from incident KeyEdges.
    //
    // Incident  InbetweenEdges are ignored  since they  do not
    // store  actual geometry: they  actually use  the Key
    // node position to computed their geometry, not the other
    // way around!
    //
    // In  the  future,  if  non-keyframed instant  edges  are
    // allowed, then it would be necessary to ignore them too.

    CellSet ss = spatialStar();

    // we don't directly modify  pos_, because in case of n==0
    // we prefer to keep the old pos_ than to replace it by 0
    Eigen::Vector2d res(0,0);

    int n = 0;
    for(Cell * c: ss)
    {
        KeyEdge * iedge = c->toKeyEdge();
        if(iedge)
        {
            if(iedge->startVertex() == this)
            {
                res += iedge->geometry()->leftPos2d();
                n++;
            }
            if(iedge->endVertex() == this)
            {
                res += iedge->geometry()->rightPos2d();
                n++;
            }
        }

    }

    if(n>0)
        setPos(res / (double) n);
}

void KeyVertex::correctEdgesGeometry()
{
    CellSet ss = spatialStar();

    for(Cell * c: ss)
    {
        KeyEdge * iedge = c->toKeyEdge();
        if(iedge)
            iedge->correctGeometry();
    }
}

Eigen::Vector2d KeyVertex::pos() const
{
    return pos_;
}

KeyVertexList KeyVertex::beforeVertices() const
{
    InbetweenVertexSet beforeAN = temporalStarBefore();
    KeyVertexList res;
    for(InbetweenVertex * node: beforeAN)
        res << node->beforeVertex();
    return res;
}

KeyVertexList KeyVertex::afterVertices() const
{
    InbetweenVertexSet afterAN = temporalStarAfter();
    KeyVertexList res;
    for(InbetweenVertex * node: afterAN)
        res << node->afterVertex();
    return res;
}

Eigen::Vector2d KeyVertex::catmullRomTangent(bool slowInOut) const
{
    Eigen::Vector3d u(0,0,0);

    KeyVertexList B = beforeVertices();
    KeyVertexList A = afterVertices();
    for(KeyVertex * node: B)
    {
        Eigen::Vector2d dp = pos() - node->pos();
        double dt = time().floatTime() - node->time().floatTime();
        u += Eigen::Vector3d(dp[0],dp[1],dt);
    }
    for(KeyVertex * node: A)
    {
        Eigen::Vector2d dp = node->pos() - pos();
        double dt = node->time().floatTime() - time().floatTime();
        u += Eigen::Vector3d(dp[0],dp[1],dt);
    }

    if(slowInOut && ( (B.size()==0) || (A.size()==0) ))
        return Eigen::Vector2d(0,0);
    else if(u[2]>0)
        return Eigen::Vector2d(u[0],u[1])/u[2];
    else
        return Eigen::Vector2d(0,0);
}

Eigen::Vector2d KeyVertex::dividedDifferencesTangent(bool slowInOut) const
{
    Eigen::Vector2d u(0,0);

    KeyVertexList B = beforeVertices();
    int n = B.size();

    KeyVertexList A = afterVertices();
    int m = A.size();

    for(KeyVertex * node: B)
    {
        Eigen::Vector2d dp = pos() - node->pos();
        double dt = time().floatTime() - node->time().floatTime();
        u += dp/dt;
    }
    for(KeyVertex * node: A)
    {
        Eigen::Vector2d dp = node->pos() - pos();
        double dt = node->time().floatTime() - time().floatTime();
        u += dp/dt;
    }

    if(slowInOut && ( !n || !m ))
        return Eigen::Vector2d(0,0);
    else if(m+n>0)
        return u/(m+n);
    else
        return Eigen::Vector2d(0,0);
}

void KeyVertex::setPos(const Eigen::Vector2d & pos)
{
    pos_ = pos;
    processGeometryChanged_();
}

void KeyVertex::prepareDragAndDrop()
{
    posBack_ = pos_;
    //todo: store edge geometry
}

void KeyVertex::performDragAndDrop(double dx, double dy)
{
    setPos(posBack_ + Eigen::Vector2d(dx,dy));
    //correctEdgesGeometry();
}

void KeyVertex::prepareAffineTransform()
{
    posBack_ = pos_;
}

void KeyVertex::performAffineTransform(const Eigen::Affine2d & xf)
{
    setPos(xf * posBack_);
}

bool KeyVertex::check_() const
{
    // todo
    return true;
}

}
