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

#include "EdgeGeometry.h"

#include "InbetweenEdge.h"
#include "KeyEdge.h"
#include "KeyVertex.h"
#include "VAC.h"
#include "Intersection.h"

#include <limits>

#include "../OpenGL.h"
#include <QTextStream>
#include "../SaveAndLoad.h"
#include "../DevSettings.h"
#include "../Global.h"

#include "../XmlStreamReader.h"
#include "../XmlStreamWriter.h"

namespace VectorAnimationComplex
{

KeyEdge::KeyEdge(VAC * vac, Time time,
         KeyVertex * startVertex,
         KeyVertex * endVertex,
         EdgeGeometry * geometry) :
    Cell(vac),
    KeyCell(vac, time),
    EdgeCell(vac),
    startVertex_(startVertex),
    endVertex_(endVertex),
    geometry_(geometry)
{
    if(startVertex_)
        addMeToSpatialStarOf_(startVertex_);
    if(endVertex_)
        addMeToSpatialStarOf_(endVertex_);

    // todo: if geometry == 0, create it, store that it has been
    // created automatically, delete it in ~KeyEdge

}

KeyEdge::KeyEdge(VAC * vac, Time time,
         EdgeGeometry * geometry) :
    Cell(vac),
    KeyCell(vac, time),
    EdgeCell(vac),
    startVertex_(0),
    endVertex_(0),
    geometry_(geometry)
{
}

void KeyEdge::save_(QTextStream & out)
{

    // Base classes
    Cell::save_(out);
    KeyCell::save_(out);
    EdgeCell::save_(out);

    // Left vertex
    out << Save::newField("StartVertex");
    if(startVertex_ )
        out << startVertex_->id();
    else
        out << -1;

    // Right vertex
    out << Save::newField("EndVertex");
    if(endVertex_ )
        out << endVertex_->id();
    else
        out << -1;

    // Geometry
    out << Save::newField("Geometry");
    out << Save::openCurlyBrackets();
    if(geometry_)
            geometry_->save(out);
    out << Save::closeCurlyBrackets();

}

KeyEdge::KeyEdge(VAC * vac, XmlStreamReader & xml)  :
    Cell(vac, xml),
    KeyCell(vac, xml),
    EdgeCell(vac, xml)
{
    tmp_ = new TempRead();

    if(xml.attributes().hasAttribute("startvertex"))
        tmp_->left = xml.attributes().value("startvertex").toInt();
    else
        tmp_->left = -1;
    if(xml.attributes().hasAttribute("endvertex"))
        tmp_->right = xml.attributes().value("endvertex").toInt();
    else
        tmp_->right = -1;

    geometry_ = EdgeGeometry::read(xml);
}

KeyEdge::KeyEdge(VAC * vac, QTextStream & in) :
    Cell(vac, in),
    KeyCell(vac, in),
    EdgeCell(vac, in)
{
    Field field;
    QString bracket;
    tmp_ = new TempRead();

    // Left node
    in >> /*field >>*/ tmp_->left; // Reason of comment: see Cell::Cell(VAC * vac, QTextStream & in)

    // Right node
    in >> field >> tmp_->right;

    // Geometry
    in >> field >> bracket;
    geometry_ = EdgeGeometry::read(in);
    in >> bracket;
}

void KeyEdge::read2ndPass()
{
    // Base classes
    Cell::read2ndPass();
    KeyCell::read2ndPass();
    EdgeCell::read2ndPass();


    // Left
    Cell * cell = getCell(tmp_->left);
    if(cell)
        startVertex_ = cell->toKeyVertex();
    else
        startVertex_ = 0;

    // Right
    cell = getCell(tmp_->right);
    if(cell)
        endVertex_ = cell->toKeyVertex();
    else
        endVertex_ = 0;

    // Geometry
    if(isClosed())
        geometry_->makeLoop();

    delete tmp_;
}

QString KeyEdge::xmlType_() const
{
    return "edge";
}

void KeyEdge::write_(XmlStreamWriter & xml) const
{
    // Base classes
    Cell::write_(xml);
    KeyCell::write_(xml);
    EdgeCell::write_(xml);

    // Start vertex
    if(startVertex_)
        xml.writeAttribute("startvertex", QString().setNum(startVertex_->id()));

    // End vertex
    if(endVertex_)
        xml.writeAttribute("endvertex", QString().setNum(endVertex_->id()));

    // Geometry
    geometry_->write(xml);
}



KeyEdge * KeyEdge::clone()
{
    return new KeyEdge(this);
}

void KeyEdge::remapPointers(VAC * newVAC)
{
    Cell::remapPointers(newVAC);
    KeyCell::remapPointers(newVAC);
    EdgeCell::remapPointers(newVAC);

    if(startVertex_)
        startVertex_ = newVAC->getCell(startVertex_->id())->toKeyVertex();
    if(endVertex_)
        endVertex_ = newVAC->getCell(endVertex_->id())->toKeyVertex();
}

// Update Boundary
void KeyEdge::updateBoundary_impl(KeyVertex * oldVertex, KeyVertex * newVertex)
{
    if(startVertex_ == oldVertex)
        startVertex_ = newVertex;
    if(endVertex_ == oldVertex)
        endVertex_ = newVertex;
}

KeyEdge::KeyEdge(KeyEdge * other) :
    Cell(other),
    KeyCell(other),
    EdgeCell(other)
{
    startVertex_ = other->startVertex_;
    endVertex_ = other->endVertex_;
    geometry_ = other->geometry_->clone();
}


KeyEdge::~KeyEdge()
{
    delete geometry_;
}

VertexCellSet KeyEdge::startVertices() const
{
    VertexCellSet res;
    if(startVertex_)
        res << startVertex_;
    return res;
}

VertexCellSet KeyEdge::endVertices() const
{
    VertexCellSet res;
    if(endVertex_)
        res << endVertex_;
    return res;
}

void KeyEdge::draw3DSmall()
{
    if(temporalStar().size() == 0)
        return;

    int lineWidth = 3;
    glColor4d(0,0,0,1);
    glLineWidth(lineWidth);
    glPushMatrix();
    glTranslated(0, 0, floatTime());

    geometry()->draw();

    glPopMatrix();
    glLineWidth(1);
}

void KeyEdge::drawPickTopology(Time time, ViewSettings & /*viewSettings*/)
{
    if (!exists(time))
        return;

    Picking::glColor(id());
    geometry()->draw(2);
}

void KeyEdge::drawRaw3D(View3DSettings & viewSettings)
{
    triangles(time()).draw3D(time(), viewSettings);
}

void KeyEdge::triangulate_(Time time, Triangles & out) const
{
    out.clear();
    if (exists(time))
        geometry()->triangulate(out);
}

void KeyEdge::triangulate_(double width, Time time, Triangles & out) const
{
    out.clear();
    if (exists(time))
        geometry()->triangulate(width, out);
}

QList<EdgeSample> KeyEdge::getSampling(Time /*time*/) const
{
    return geometry()->edgeSampling();
}

void KeyEdge::correctGeometry()
{
    if(geometry())
    {
        if(isClosed())
        {
            // Fast hack to call linearSpline->curve()->resample(true).
            // will not actually change the start and end position
            geometry()->makeLoop();
            geometry()->setLeftRightPos(Eigen::Vector2d(0,0), Eigen::Vector2d(0,0));
        }
        else
        {
            geometry()->setLeftRightPos(startVertex()->pos(), endVertex()->pos());
        }

        processGeometryChanged_();
    }
}

void KeyEdge::setWidth(double newWidth)
{
    geometry()->setWidth(newWidth);
    processGeometryChanged_();
}

double KeyEdge::updateSculpt(double x, double y, double radius)
{
    sculptRadius_ = radius;
    double res = geometry()->updateSculpt(x, y, radius);
    remainingRadiusLeft_ = radius - geometry()->arclengthOfSculptVertex();
    if(remainingRadiusLeft_ < 0)
        remainingRadiusLeft_ = 0;
    remainingRadiusRight_ = radius - ( geometry()->length() - geometry()->arclengthOfSculptVertex() );
    if(remainingRadiusRight_ < 0)
        remainingRadiusRight_ = 0;
    return res;
}

void KeyEdge::beginSculptDeform(double x, double y)
{
    // prepare geometry for sculpting
    geometry()->beginSculptDeform(x, y);
    prepareSculptPreserveTangents_();
}

void KeyEdge::prepareSculptPreserveTangents_()
{
    const bool preserveTangentEdges = false;
    if (!preserveTangentEdges)
        return;


    // find instant edges where tangency must be preserved
    sculpt_beginLeftDer_ = geometry()->der(0);
    sculpt_beginRightDer_ = geometry()->der(geometry()->length());
    sculpt_keepRightAsLeft_.clear();
    sculpt_keepLeftAsLeft_.clear();
    sculpt_keepLeftAsRight_.clear();
    sculpt_keepRightAsRight_.clear();
    sculpt_keepMyselfTangent_ = false;
    if (preserveTangentEdges)
    {
        double dotThreshold = 0.9;
        if(startVertex_)
        {
            KeyEdgeSet leftEdges =  startVertex_->spatialStar();
            leftEdges.remove(this);
            for(KeyEdge * ie: leftEdges)
            {
                if(ie->endVertex_ == startVertex_)
                {
                    Eigen::Vector2d rightDer = ie->geometry()->der(ie->geometry()->length());
                    double dot = rightDer.dot(sculpt_beginLeftDer_);
                    if(dot > dotThreshold)
                        sculpt_keepRightAsLeft_ << ie;
                }
                if(ie->startVertex_ == startVertex_)
                {
                    Eigen::Vector2d rightDer = - ie->geometry()->der(0);
                    double dot = rightDer.dot(sculpt_beginLeftDer_);
                    if(dot > dotThreshold)
                        sculpt_keepLeftAsLeft_ << ie;
                }
            }
        }
        if(endVertex_)
        {
            KeyEdgeSet rightEdges =  endVertex_->spatialStar();
            rightEdges.remove(this);
            for(KeyEdge * ie: rightEdges)
            {
                // For now, disabling tangent preservation for 0-edge loops
                if(ie == this)
                    continue;

                if(ie->startVertex_ == endVertex_)
                {
                    Eigen::Vector2d leftDer = ie->geometry()->der(0);
                    double dot = leftDer.dot(sculpt_beginRightDer_);
                    if(dot > dotThreshold)
                        sculpt_keepLeftAsRight_ << ie;
                }
                if(ie->endVertex_ == endVertex_)
                {
                    Eigen::Vector2d leftDer = - ie->geometry()->der(ie->geometry()->length());
                    double dot = leftDer.dot(sculpt_beginRightDer_);
                    if(dot > dotThreshold)
                        sculpt_keepRightAsRight_ << ie;
                }
            }
        }
        if(endVertex_ && (endVertex_ == startVertex_))
        {
            // For now, disabling tangent preservation for 0-edge loops
            // What's below doesn't work, because continueSculpt assume
            // no resampling has occured since prepareSculpt

            //double dot = sculpt_beginRightDer_.dot(sculpt_beginLeftDer_);
            //if(dot > dotThreshold)
            //{
            //    sculpt_keepMyselfTangent_ = true;
            //}
        }
    }
}

void KeyEdge::continueSculptDeform(double x, double y)
{
    geometry()->continueSculptDeform(x, y);
    processGeometryChanged_();
    continueSculptPreserveTangents_();
}

void KeyEdge::continueSculptPreserveTangents_()
{
    const bool preserveTangentEdges = false;
    if (!preserveTangentEdges)
        return;

    // preserve tangency
    Eigen::Vector2d continueLeftDer = geometry()->der(0);
    Eigen::Vector2d continueRightDer = geometry()->der(geometry()->length());
    for(KeyEdge * ie: sculpt_keepRightAsLeft_)
        ie->geometry()->setRightDer(continueLeftDer, remainingRadiusLeft_, true);
    for(KeyEdge * ie: sculpt_keepLeftAsLeft_)
        ie->geometry()->setLeftDer(-continueLeftDer, remainingRadiusLeft_, true);
    for(KeyEdge * ie: sculpt_keepLeftAsRight_)
        ie->geometry()->setLeftDer(continueRightDer, remainingRadiusRight_, true);
    for(KeyEdge * ie: sculpt_keepRightAsRight_)
        ie->geometry()->setRightDer(-continueRightDer, remainingRadiusRight_, true);
    if(sculpt_keepMyselfTangent_)
    {
        geometry()->setRightDer(continueLeftDer, remainingRadiusLeft_, false);
        geometry()->setLeftDer(continueRightDer, remainingRadiusRight_, false);

    }
}

void KeyEdge::endSculptDeform()
{
    geometry()->endSculptDeform();
    processGeometryChanged_();
}

void KeyEdge::beginSculptEdgeWidth(double x, double y)
{
    geometry()->beginSculptEdgeWidth(x, y);
}

void KeyEdge::continueSculptEdgeWidth(double x, double y)
{
    geometry()->continueSculptEdgeWidth(x, y);
    processGeometryChanged_();
}

void KeyEdge::endSculptEdgeWidth()
{
    geometry()->endSculptEdgeWidth();
    processGeometryChanged_();
}

void KeyEdge::beginSculptSmooth(double x, double y)
{
    geometry()->beginSculptSmooth(x, y);
    //prepareSculptPreserveTangents_(); // doesn't make sense since sculpt vertex can be different in continueSculptSmooth
}

void KeyEdge::continueSculptSmooth(double x, double y)
{
    prepareSculptPreserveTangents_();
    geometry()->continueSculptSmooth(x, y);
    processGeometryChanged_();
    //correctGeometry(); // now ensured by geometry()->continueSculptSmooth(x, y)
    continueSculptPreserveTangents_();
}

void KeyEdge::endSculptSmooth()
{
    geometry()->endSculptSmooth();
    processGeometryChanged_();
}

void KeyEdge::prepareAffineTransform()
{
    geometry()->prepareAffineTransform();
}

void KeyEdge::performAffineTransform(const Eigen::Affine2d & xf)
{
    geometry()->performAffineTransform(xf);
    processGeometryChanged_();
}

bool KeyEdge::check_() const
{
    // todo
    return true;
}

}
