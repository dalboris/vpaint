// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "View2D.h"

#include "Scene/SceneRenderer.h"
#include "Scene/Scene.h"
#include "Views/TestAction.h"
#include "Views/View2DRenderer.h"
#include "Tools/Sketch/SketchAction.h"

View2D::View2D(Scene *scene,
               SceneRendererSharedResources * sceneRendererSharedResources,
               QWidget * parent):

    View(parent),

    scene_(scene), // XXX move to View?
    sceneRendererSharedResources_(sceneRendererSharedResources)
{
    createRenderers_();
    addActions_();
    updateViewOnSceneChange_(); // XXX move to View?
}

View2DMouseEvent * View2D::makeMouseEvent()
{
    return new View2DMouseEvent(this);
}

void View2D::createRenderers_()
{
    sceneRenderer_ = new SceneRenderer(sceneRendererSharedResources_, this);
    view2DRenderer_ = new View2DRenderer(sceneRenderer_, this);
    setRenderer(view2DRenderer_);

}

void View2D::addActions_()
{
    addMouseAction(new TestAction(scene_));
    addMouseAction(new SketchAction(scene_));
}

void View2D::updateViewOnSceneChange_() // XXX move to View?
{
    // Note: This is just a regular signal/slot connection, but we need a
    // static_cast here to resolve overload ambiguity, i.e. to tell the compiler
    // to use 'update()' and not, for instance, 'update(const QRect &)'.

    // Type of 'void QWidget::update()'
    using update_t = void (QWidget::*) ();

    // Create connection
    connect(scene_, &Scene::changed, this, static_cast<update_t>(&QWidget::update));
}
