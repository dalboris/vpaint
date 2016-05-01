// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "SketchAction.h"

#include <QtDebug>

SketchAction::SketchAction(Scene * scene) :
    scene_(scene)
{
}

bool SketchAction::acceptPMREvent(const View2DMouseEvent * event)
{
    qDebug() << "SketchAction::acceptPMREvent";

    return event->button() == Qt::LeftButton;
}

void SketchAction::pressEvent(const View2DMouseEvent * event)
{
    qDebug() << "SketchAction::pressEvent";
}

void SketchAction::moveEvent(const View2DMouseEvent * event)
{
    qDebug() << "SketchAction::moveEvent";
}

void SketchAction::releaseEvent(const View2DMouseEvent * event)
{
    qDebug() << "SketchAction::releaseEvent";
}
