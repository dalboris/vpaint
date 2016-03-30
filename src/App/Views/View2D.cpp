// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "View2D.h"

#include "Views/TestAction.h"
#include "Views/View2DRenderer.h"
#include "Scene/SceneRenderer.h"
#include "Scene/Scene.h"

#define SLOT_(Class, SlotName, Params) \
    static_cast<void (Class::*) (Params)> (&Class::SlotName)

View2D::View2D(SceneRendererSharedResources * sceneRendererSharedResources, QWidget * parent):
    View(parent)
{
    sceneRenderer_ = new SceneRenderer(sceneRendererSharedResources, this);
    view2DRenderer_ = new View2DRenderer(sceneRenderer_, this);
    setRenderer(view2DRenderer_);

    Scene * scene = sceneRenderer_->scene();

    addMouseAction(new TestAction(scene));

    connect(scene, &Scene::changed, this, SLOT_(QWidget, update, ));
}

View2DMouseEvent * View2D::makeMouseEvent()
{
    return new View2DMouseEvent(this);
}
