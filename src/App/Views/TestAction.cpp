// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "TestAction.h"

#include "Scene/Scene.h"

TestAction::TestAction(Scene * scene):
    scene_(scene)
{

}

void TestAction::pressEvent(const View2DMouseEvent * event)
{
    qDebug() << "TestAction pressEvent" << event->viewPos();

    scene_->data_.clear();
    scene_->data_.addSample(glm::vec2(
                                (float) event->scenePos().x(),
                                (float) event->scenePos().y()));
}

void TestAction::moveEvent(const View2DMouseEvent * event)
{
    qDebug() << "TestAction moveEvent" << event->viewPos();

    scene_->data_.addSample(glm::vec2(
                                (float) event->scenePos().x(),
                                (float) event->scenePos().y()));
}

void TestAction::releaseEvent(const View2DMouseEvent * event)
{
    qDebug() << "TestAction releaseEvent" << event->viewPos();
}
