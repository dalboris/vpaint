// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "View2DMouseEvent.h"

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
    scenePos_ = computeScenePos_(viewPos());
}

void View2DMouseEvent::computeSceneAttributesAtPress()
{
    scenePosAtPress_ = computeScenePos_(scenePosAtPress());
}

QPointF View2DMouseEvent::computeScenePos_(const QPointF & viewPos)
{
    // XXX TODO
    return QPointF(0.0, 0.0);


    /*
    // convert to scene coordinates
    if(isOnly2D_)
    {
        Eigen::Vector3d p = camera2D_.viewMatrixInverse() * Eigen::Vector3d(mouse_PressEvent_X_, mouse_PressEvent_Y_, 0);
        mouse_PressEvent_XScene_ = p[0];
        mouse_PressEvent_YScene_ = p[1];
    }
    */

}
