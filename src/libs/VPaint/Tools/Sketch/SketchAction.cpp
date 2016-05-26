// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "SketchAction.h"

#include "Scene/Scene.h"
#include "Views/View2D.h"

#include "OpenVac/Topology/KeyEdge.h"
#include "OpenVac/Operators/MakeKeyVertex.h"
#include "OpenVac/Operators/MakeKeyOpenEdge.h"

#include <glm/vec2.hpp>

namespace Op = OpenVac::Operators;

struct SketchActionBegin: public DataObjectMutator<VacData>
{
    OpenVac::KeyEdgeHandle edge;
    VecCurveInputSample inputSample;

    SketchActionBegin(
            const VecCurveInputSample & inputSample) :
        inputSample(inputSample)
    {
    }

    void exec(VacData & vac)
    {
        OpenVac::Geometry::Frame frame = 0;

        vac.beginTopologyEdit();
        auto startVertex = Op::makeKeyVertex(vac, frame);
        auto endVertex   = Op::makeKeyVertex(vac, frame);
        edge             = Op::makeKeyOpenEdge(startVertex, endVertex);
        vac.endTopologyEdit();

        vac.beginGeometryEdit(edge);
        edge->geometry().beginFit();
        edge->geometry().addFitInputSample(inputSample);
        vac.endGeometryEdit();
    }
};

struct SketchActionContinue: public DataObjectMutator<VacData>
{
    OpenVac::KeyEdgeHandle edge;
    VecCurveInputSample inputSample;

    SketchActionContinue(
            const OpenVac::KeyEdgeHandle & edge,
            const VecCurveInputSample & inputSample) :
        edge(edge),
        inputSample(inputSample)
    {
    }

    void exec(VacData & vac)
    {
        vac.beginGeometryEdit(edge);
        edge->geometry().addFitInputSample(inputSample);
        vac.endGeometryEdit();
    }
};

struct SketchActionEnd: public DataObjectMutator<VacData>
{
    OpenVac::KeyEdgeHandle edge;

    SketchActionEnd(
             const OpenVac::KeyEdgeHandle & edge) :
        edge(edge)
    {
    }

    void exec(VacData & vac)
    {
        vac.beginGeometryEdit(edge);
        edge->geometry().endFit();
        vac.endGeometryEdit();
    }
};

SketchAction::SketchAction(Scene * scene) :
    scene_(scene)
{
}

bool SketchAction::acceptPMREvent(const View2DMouseEvent * event)
{
    return (event->modifiers() == Qt::NoModifier) &&
           (event->button() == Qt::LeftButton);
}

VecCurveInputSample SketchAction::getInputSample_(const View2DMouseEvent * event)
{
    const glm::vec2 position((float) event->scenePos().x(),
                            (float) event->scenePos().y());

    const double width = 10.0; // XXX TODO
    const double time = event->timeSincePress();
    const double resolution = 1.0 / event->view()->camera()->scale();

    return VecCurveInputSample(position, width, time, resolution);
}

void SketchAction::pressEvent(const View2DMouseEvent * event)
{
    SketchActionBegin m(getInputSample_(event));
    scene_->activeVac()->accept(m);
    edge = m.edge;
}

void SketchAction::moveEvent(const View2DMouseEvent * event)
{
    SketchActionContinue m(edge, getInputSample_(event));
    scene_->activeVac()->accept(m);
}

void SketchAction::releaseEvent(const View2DMouseEvent * /*event*/)
{
    SketchActionEnd m(edge);
    scene_->activeVac()->accept(m);
}
