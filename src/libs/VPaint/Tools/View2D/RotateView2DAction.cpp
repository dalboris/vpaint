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
    connect(&resetAnimationTimer_, &QTimer::timeout, this, &RotateView2DAction::onTimeout_);
}

bool RotateView2DAction::accept_(const View2DMouseEvent * event)
{
    // Complete animation in case animation is not finished
    // when any new mouse action is performed
    completeAnimation_();

    // Accept event if ALT + MLB
    return (event->modifiers() & Qt::AltModifier) &&
           (event->button() == Qt::LeftButton);
}

bool RotateView2DAction::acceptPMREvent(const View2DMouseEvent * event)
{
    return accept_(event);
}

bool RotateView2DAction::acceptClickEvent(const View2DMouseEvent * event)
{
    return accept_(event);
}

void RotateView2DAction::storeDataAtPress_(const View2DMouseEvent * event)
{
    cameraDataAtPress_ = camera2D_->data();
    scenePivot_ = event->scenePosAtPress();
    viewPivot_ = event->viewPosAtPress();
}

void RotateView2DAction::pressEvent(const View2DMouseEvent * event)
{
    storeDataAtPress_(event);
}

void RotateView2DAction::moveEvent(const View2DMouseEvent * event)
{
    // Compute new rotation based on mouse offset
    double dx = (event->viewPos().x() - event->viewPosAtPress().x());
    double dy = (event->viewPos().y() - event->viewPosAtPress().y());
    double dr = 0.01 * (dy - dx); // XXX This should be a user preference:
                                  //          a*dx + b*dy
                                  // where a,b = sensitivity (> 0, == 0, or < 0)
    double rotation = cameraDataAtPress_.rotation + dr;

    // Set rotation
    setRotation_(rotation);
}

void RotateView2DAction::releaseEvent(const View2DMouseEvent * /*event*/)
{
}

namespace
{
// Returns the equivalent rotation which is in the [-pi, pi] range.
// Preserves sign in case of the ambiguity between -pi or pi.
double equivalentRotationClosestToZero_(double rotation)
{
    // Rotation in (-2*pi, 2*pi), same sign
    double res = std::fmod(rotation, 2*M_PI);

    // Rotation in [-pi,pi]
    if (res < -M_PI)
        res += 2*M_PI;
    else if (res > M_PI)
        res -= 2*M_PI;

    // Return canonical rotation
    return res;
}
}

void RotateView2DAction::clickEvent(const View2DMouseEvent * event)
{
    // Store data at press
    storeDataAtPress_(event);

    // Move rotation at press to the [-pi, pi] range ----
    cameraDataAtPress_.rotation =
            equivalentRotationClosestToZero_(cameraDataAtPress_.rotation);

    // Start timer
    resetAnimationElapsedTimer_.start();
    resetAnimationTimer_.start(16); // 16ms
}

namespace
{
double easeOut_(double x)
{
    return (1.0 - x) * (1.0 - x);
}
}

void RotateView2DAction::onTimeout_()
{
    // Duration of the animation in msec
    const double duration = 500; // 1/2 sec

    // Elapsed time since animation started, in msec
    const double elapsed = resetAnimationElapsedTimer_.elapsed();

    // Is animation finished yet?
    if (elapsed < duration)
    {
        // If not finished, set rotation and wait for next timeout
        double u = elapsed / duration;
        double rotation =  cameraDataAtPress_.rotation * easeOut_(u);
        setRotation_(rotation);
    }
    else
    {
        // If finished, stop timer and set rotation to 0
        completeAnimation_();
    }
}

void RotateView2DAction::setRotation_(double rotation)
{
    // Initialize new camera data to camera data at press
    Camera2DData newCameraData = cameraDataAtPress_;

    // Set new rotation
    newCameraData.rotation = rotation;

    // Set new position so that the mouse position is the pivot point
    newCameraData.translateScenePosToViewPos(scenePivot_, viewPivot_);

    // Set new camera data (this emits Camera2D::changed())
    camera2D_->setData(newCameraData);
}

void RotateView2DAction::completeAnimation_()
{
    if (resetAnimationTimer_.isActive())
    {
        resetAnimationTimer_.stop();
        setRotation_(0);
    }
}
