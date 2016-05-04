// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "RotateView2DAction.h"

#include "Cameras/Camera2D.h"

#include <cmath>

RotateView2DAction::RotateView2DAction(Camera2D * camera2D) :
    camera2D_(camera2D)
{
}

bool RotateView2DAction::acceptPMREvent(const View2DMouseEvent * event)
{
    return (event->modifiers() & Qt::AltModifier) &&
           (event->button() == Qt::LeftButton);
}

void RotateView2DAction::pressEvent(const View2DMouseEvent * /*event*/)
{
    cameraDataAtPress_ = camera2D_->data();
}

void RotateView2DAction::moveEvent(const View2DMouseEvent * event)
{
    // Initialize new data to old data
    Camera2DData newCameraData = cameraDataAtPress_;

    // Compute rotation offset based on mouse offset
    double dx = (event->viewPos().x() - event->viewPosAtPress().x());
    double dy = (event->viewPos().y() - event->viewPosAtPress().y());
    double dr = 0.01 * (dy - dx); // XXX This should be a user preference:
                                  //          a*dx + b*dy
                                  // where a,b = sensitivity (> 0, == 0, or < 0)

    // Sets new rotation
    newCameraData.rotation += dr;

    // Sets new position so that the mouse position is the pivot point
    newCameraData.translateScenePosToViewPos(event->scenePosAtPress(),
                                             event->viewPosAtPress());

    // Set new camera data
    camera2D_->setData(newCameraData);
}

void RotateView2DAction::releaseEvent(const View2DMouseEvent * /*event*/)
{
}
