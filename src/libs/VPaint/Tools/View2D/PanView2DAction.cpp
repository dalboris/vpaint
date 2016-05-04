// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "PanView2DAction.h"

#include "Cameras/Camera2D.h"

PanView2DAction::PanView2DAction(Camera2D * camera2D) :
    camera2D_(camera2D)
{
}

bool PanView2DAction::acceptPMREvent(const View2DMouseEvent * event)
{
    return (event->modifiers() & Qt::AltModifier) &&
           (event->button() == Qt::MidButton);
}

void PanView2DAction::pressEvent(const View2DMouseEvent * /*event*/)
{
    cameraDataAtPress_ = camera2D_->data();
}

void PanView2DAction::moveEvent(const View2DMouseEvent * event)
{
    Camera2DData newCameraData = cameraDataAtPress_;

    newCameraData.translateScenePosToViewPos(
                event->scenePosAtPress(),
                event->viewPos());

    camera2D_->setData(newCameraData);
}

void PanView2DAction::releaseEvent(const View2DMouseEvent * /*event*/)
{
}
