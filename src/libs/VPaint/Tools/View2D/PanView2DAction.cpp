// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "PanView2DAction.h"

#include "Cameras/Camera2D.h"

#include <QtDebug>

PanView2DAction::PanView2DAction(Camera2D * camera2D) :
    camera2D_(camera2D)
{
}

bool PanView2DAction::acceptPMREvent(const View2DMouseEvent * event)
{
    qDebug() << "PanView2DAction::acceptPMREvent";

    return event->button() == Qt::MidButton;
}

void PanView2DAction::pressEvent(const View2DMouseEvent * event)
{
    qDebug() << "PanView2DAction::pressEvent";

    cameraPositionAtPress_ = camera2D_->position();
}

void PanView2DAction::moveEvent(const View2DMouseEvent * event)
{
    qDebug() << "PanView2DAction::moveEvent" << event->viewPos();


    QPointF deltaPosition = event->viewPos() - event->viewPosAtPress();
    QPointF newCameraPosition = cameraPositionAtPress_ + deltaPosition;

    camera2D_->setPosition(newCameraPosition);
}

void PanView2DAction::releaseEvent(const View2DMouseEvent * event)
{
    qDebug() << "PanView2DAction::releaseEvent";
}
