// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "Views/View2DMouseEvent.h"
#include "Views/View2D.h"
#include "OpenGL/OpenGLRenderer.h"

View2DMouseEvent::View2DMouseEvent(View2D * view2D) :
    ViewMouseEvent(),
    view2D_(view2D)
{
}

QPointF View2DMouseEvent::scenePos() const
{
    return scenePos_;
}

QPointF View2DMouseEvent::scenePosAtPress() const
{
    return scenePosAtPress_;
}

void View2DMouseEvent::computeSceneAttributes()
{
    scenePos_ = view2D_->mapToScene(viewPos());
}

void View2DMouseEvent::computeSceneAttributesAtPress()
{
    scenePosAtPress_ = view2D_->mapToScene(viewPosAtPress());
}
