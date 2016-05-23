// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "View2D.h"

#include "Scene/SceneRenderer.h"
#include "Views/View2DRenderer.h"

#include "Tools/Sketch/SketchAction.h"
#include "Tools/View2D/PanView2DAction.h"
#include "Tools/View2D/RotateView2DAction.h"
#include "Tools/View2D/ZoomView2DAction.h"

View2D::View2D(Scene * scene,
               SceneRendererSharedResources * sceneRendererSharedResources,
               QWidget * parent):

    View(scene, parent),

    sceneRendererSharedResources_(sceneRendererSharedResources),
    camera2D_()
{
    setRenderer(new View2DRenderer(sceneRendererSharedResources, camera()));
    addActions_();
    connect(camera2D_.get(), &Camera2D::changed, this, &View2D::repaint);
}

View2D::~View2D()
{
}

SceneRendererSharedResources * View2D::sceneRendererSharedResources() const
{
    return sceneRendererSharedResources_;
}

Camera2D * View2D::camera() const
{
    return camera2D_.get();
}

QPointF View2D::mapToScene(const QPointF & viewPos) const
{
    return camera2D_->toMatrix().inverted() * viewPos;
}

View2DMouseEvent * View2D::makeMouseEvent()
{
    return new View2DMouseEvent(this);
}

void View2D::addActions_()
{
    addMouseAction(new SketchAction(scene()));
    addMouseAction(new PanView2DAction(camera()));
    addMouseAction(new RotateView2DAction(camera()));
    addMouseAction(new ZoomView2DAction(camera()));
}
