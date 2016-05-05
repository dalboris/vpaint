// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "SketchAction.h"

#include "Scene/Scene.h"

#include "OpenVac/Topology/KeyEdge.h"
#include "OpenVac/Operators/MakeKeyVertex.h"
#include "OpenVac/Operators/MakeKeyOpenEdge.h"

#include <glm/vec2.hpp>

namespace Op = OpenVac::Operators;

struct SketchActionBegin: public DataObjectMutator<VacData>
{
    OpenVac::KeyEdgeHandle edge;
    glm::vec2 centerline;

    void exec(VacData & vac)
    {
        OpenVac::Geometry::Frame frame = 0;

        auto startKeyVertex = Op::makeKeyVertex(vac, frame);
        auto endKeyVertex   = Op::makeKeyVertex(vac, frame);

        edge = Op::makeKeyOpenEdge(startKeyVertex, endKeyVertex);
        edge->geometry().beginStroke(centerline);
    }
};

struct SketchActionContinue: public DataObjectMutator<VacData>
{
    OpenVac::KeyEdgeHandle edge;
    glm::vec2 centerline;

    void exec(VacData & /*vac*/)
    {
        edge->geometry().continueStroke(centerline);
    }
};

struct SketchActionEnd: public DataObjectMutator<VacData>
{
    OpenVac::KeyEdgeHandle edge;

    void exec(VacData & /*vac*/)
    {
        edge->geometry().endStroke();
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

void SketchAction::pressEvent(const View2DMouseEvent * event)
{
    glm::vec2 centerline((float) event->scenePos().x(),
                         (float) event->scenePos().y());

    SketchActionBegin m;
    m.centerline = centerline;
    scene_->activeVac()->accept(m);
    edge = m.edge;
}

void SketchAction::moveEvent(const View2DMouseEvent * event)
{
    glm::vec2 centerline((float) event->scenePos().x(),
                         (float) event->scenePos().y());

    SketchActionContinue m;
    m.edge = edge;
    m.centerline = centerline;
    scene_->activeVac()->accept(m);
}

void SketchAction::releaseEvent(const View2DMouseEvent * /*event*/)
{
    SketchActionEnd m;
    m.edge = edge;
    scene_->activeVac()->accept(m);
}
