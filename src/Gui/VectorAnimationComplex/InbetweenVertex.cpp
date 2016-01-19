// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "InbetweenVertex.h"
#include "KeyVertex.h"

#include "VAC.h"

#include "../OpenGL.h"
#include "../SaveAndLoad.h"
#include "../DevSettings.h"
#include "../Global.h"

#include <QtDebug>
#include <QTextStream>

#include "../XmlStreamReader.h"
#include "../XmlStreamWriter.h"

namespace VectorAnimationComplex
{

// Constructor
InbetweenVertex::InbetweenVertex(VAC * vac,
                   KeyVertex * beforeVertex,
                   KeyVertex * afterVertex):
    Cell(vac),
    InbetweenCell(vac),
    VertexCell(vac),

    beforeVertex_(beforeVertex),
    afterVertex_(afterVertex)
{
    // color
    color_[0] = 0;
    color_[1] = 0;
    color_[2] = 0;
    color_[3] = 1;

    // add the boundary to this
    addMeToTemporalStarAfterOf_(beforeVertex_);
    addMeToTemporalStarBeforeOf_(afterVertex_);
}

InbetweenVertex::~InbetweenVertex()
{
}

InbetweenVertex::InbetweenVertex(VAC * vac, QTextStream & in) :
    Cell(vac, in),
    InbetweenCell(vac, in),
    VertexCell(vac, in),

    beforeVertex_(0),
    afterVertex_(0)
{
    color_[0] = 0;
    color_[1] = 0;
    color_[2] = 0;
    color_[3] = 1;

    Field field;
    tmp_ = new TempRead();

    // Before node / After node
    in >> field >> tmp_->before;
    in >> field >> tmp_->after;
}
void InbetweenVertex::read2ndPass()
{
    // Base classes
    Cell::read2ndPass();
    InbetweenCell::read2ndPass();
    VertexCell::read2ndPass();

    // Before node / After node
    beforeVertex_ = getCell(tmp_->before)->toKeyVertex();
    afterVertex_ = getCell(tmp_->after)->toKeyVertex();

    delete tmp_;
}


void InbetweenVertex::save_(QTextStream & out)
{
    // Base classes
    Cell::save_(out);
    InbetweenCell::save_(out);
    VertexCell::save_(out);

    // Before node / After node
    out << Save::newField("BeforeVertex") << beforeVertex_->id();
    out << Save::newField("AfterVertex") << afterVertex_->id();
}

QString InbetweenVertex::xmlType_() const
{
    return "inbetweenvertex";

}
void InbetweenVertex::write_(XmlStreamWriter & xml) const
{
    // Base classes
    Cell::write_(xml);
    InbetweenCell::write_(xml);
    VertexCell::write_(xml);

    // Before/After Vertices
    xml.writeAttribute("beforevertex", QString().setNum(beforeVertex_->id()));
    xml.writeAttribute("aftervertex", QString().setNum(afterVertex_->id()));
}

InbetweenVertex::InbetweenVertex(VAC * vac, XmlStreamReader & xml) :
    Cell(vac, xml),
    InbetweenCell(vac, xml),
    VertexCell(vac, xml),

    beforeVertex_(0),
    afterVertex_(0)
{
    color_[0] = 0;
    color_[1] = 0;
    color_[2] = 0;
    color_[3] = 1;

    tmp_ = new TempRead();
    tmp_->before = xml.attributes().value("beforevertex").toInt();
    tmp_->after = xml.attributes().value("aftervertex").toInt();
}

InbetweenVertex * InbetweenVertex::clone()
{
    return new InbetweenVertex(this);
}

void InbetweenVertex::remapPointers(VAC * newVAC)
{
    Cell::remapPointers(newVAC);
    InbetweenCell::remapPointers(newVAC);
    VertexCell::remapPointers(newVAC);

    // no pointers in this class
    beforeVertex_ = newVAC->getCell(beforeVertex_->id())->toKeyVertex();
    afterVertex_ = newVAC->getCell(afterVertex_->id())->toKeyVertex();
}


InbetweenVertex::InbetweenVertex(InbetweenVertex * other):
    Cell(other),
    InbetweenCell(other),
    VertexCell(other),

    beforeVertex_(other->beforeVertex_),
    afterVertex_(other->afterVertex_)
{
}

KeyCellSet InbetweenVertex::beforeCells() const
{
    KeyCellSet res;
    res << beforeVertex();
    return res;
}

KeyCellSet InbetweenVertex::afterCells() const
{
    KeyCellSet res;
    res << afterVertex();
    return res;
}

KeyVertex * InbetweenVertex::beforeVertex() const
{
    return beforeVertex_;
}

KeyVertex * InbetweenVertex::afterVertex() const
{
    return afterVertex_;
}

// Update Boundary
void InbetweenVertex::updateBoundary_impl(KeyVertex * oldVertex, KeyVertex * newVertex)
{
    if(beforeVertex_ == oldVertex)
        beforeVertex_ = newVertex;
    if(afterVertex_ == oldVertex)
        afterVertex_ = newVertex;
}

// Drawing

void InbetweenVertex::glColor3D_()
{
    glColor4d(0,0,0,1);
}
void InbetweenVertex::drawRaw3D(View3DSettings & viewSettings)
{
    double eps = 1e-5;

    int k = 5; // number of samples per frame
    double t1 = beforeTime().floatTime();
    double t2 = afterTime().floatTime();
    double dt = 1 / (double) k;

    int lineWidth = 3;
    glLineWidth(lineWidth);
    glBegin(GL_LINE_STRIP);
    for(double t=t1; t<t2+eps; t+=dt)
    {
        Eigen::Vector2d p = pos(Time(t));
        double x = viewSettings.xFromX2D(p[0]);
        double y = viewSettings.yFromY2D(p[1]);
        double z = viewSettings.zFromT(t);
        glVertex3d(x,y,z);
    }
    glEnd();

    glLineWidth(1);
}

// Geometry
Eigen::Vector2d InbetweenVertex::pos(Time time) const
{
    return posCubic(time);
}

// --------- Cubic spline interpolation---------

Eigen::Vector2d InbetweenVertex::posCubic(Time time) const
{
    // a notation like var_ means it is in the [t1,t2] domain
    // a notation like var  means it is in the [0 ,1 ] domain

    // --- get parameters in the [t1,t2] domain ---

    Eigen::Vector2d p1_ = beforeVertex()->pos();
    //Eigen::Vector2d m1_ = beforeVertex()->catmullRomTangent(true);
    //Eigen::Vector2d m1_ = beforeVertex()->dividedDifferencesTangent(true);
    Eigen::Vector2d m1_ = beforeVertex()->dividedDifferencesTangent(false);
    double t1_ = beforeVertex()->time().floatTime();

    Eigen::Vector2d p2_ = afterVertex()->pos();
    //Eigen::Vector2d m2_ = afterVertex()->catmullRomTangent(true);
    //Eigen::Vector2d m2_ = afterVertex()->dividedDifferencesTangent(true);
    Eigen::Vector2d m2_ = afterVertex()->dividedDifferencesTangent(false);
    double t2_ = afterVertex()->time().floatTime();

    double t_ = time.floatTime();
    double dt_ = t2_-t1_;


    // --- convert in the [0,1] domain ---

    double t;
    if(dt_ > 0)      t = (t_-t1_)/dt_;
    else if(t_<t1_)  t = 0;
    else             t = 1;

    Eigen::Vector2d p1 = p1_;
    Eigen::Vector2d p2 = p2_;
    Eigen::Vector2d m1 = m1_ * dt_;
    Eigen::Vector2d m2 = m2_ * dt_;


    // --- compute cubic Hermite ---

    double t2 = t*t;
    double t3 = t2*t;
    return (2*t3-3*t2+1) * p1
        + (t3-2*t2+t) * m1
        + (-2*t3+3*t2) * p2
        + (t3-t2) * m2;
}

Eigen::Vector2d InbetweenVertex::posLinear(Time time) const
{
    double t = time.floatTime();

    double t1 = beforeVertex()->time().floatTime();
    double t2 = afterVertex()->time().floatTime();
    double dt = t2-t1;

    Eigen::Vector2d p1 = beforeVertex()->pos();
    Eigen::Vector2d p2 = afterVertex()->pos();

    if(dt>0)
        return p1 + (t-t1)/dt * (p2-p1);
    else if (t<t1)
        return p1;
    else
        return p2;
}

bool InbetweenVertex::check_() const
{
    // todo
    return true;
}

}
