// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "ZoomView2DAction.h"

#include "Cameras/Camera2D.h"

#include <cmath>

ZoomView2DAction::ZoomView2DAction(Camera2D * camera2D) :
    camera2D_(camera2D)
{
}

bool ZoomView2DAction::acceptPMREvent(const View2DMouseEvent * event)
{
    return (event->modifiers() & Qt::AltModifier) &&
           (event->button() == Qt::RightButton);
}

void ZoomView2DAction::pressEvent(const View2DMouseEvent * /*event*/)
{
    cameraDataAtPress_ = camera2D_->data();
}

void ZoomView2DAction::moveEvent(const View2DMouseEvent * event)
{
    // Initialize new data to old data
    Camera2DData newCameraData = cameraDataAtPress_;

    // Compute scale ratio based on mouse offset
    const double dx = (event->viewPos().x() - event->viewPosAtPress().x());
    const double dy = (event->viewPos().y() - event->viewPosAtPress().y());
    const double s = std::exp(0.005 * (dx - dy)); // XXX This should be a user preference:
                                                  //            exp(a*dx + b*dy)
                                                  // where a,b = sensitivity (> 0, == 0, or < 0)

    // Sets new scale
    newCameraData.scale *= s;

    // Sets new position so that the mouse position is the pivot point
    newCameraData.translateScenePosToViewPos(event->scenePosAtPress(),
                                             event->viewPosAtPress());

    // Set new camera data
    camera2D_->setData(newCameraData);
}

void ZoomView2DAction::releaseEvent(const View2DMouseEvent * /*event*/)
{
}
