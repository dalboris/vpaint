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

View2D::View2D(SceneRenderer * sceneRenderer, QWidget * parent):
    View(parent)
{
    view2DRenderer_ = new View2DRenderer(sceneRenderer, this);
    setRenderer(view2DRenderer_);

    Scene * scene = sceneRenderer->scene();

    addMouseAction(new TestAction(scene));
}

View2DMouseEvent * View2D::makeMouseEvent()
{
    return new View2DMouseEvent(this);
}
