// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef PANVIEW2DACTION_H
#define PANVIEW2DACTION_H

#include "Views/View2DMouseAction.h"

#include <QPointF>

class Camera2D;

class PanView2DAction: public View2DMouseAction
{
public:
    PanView2DAction(Camera2D * camera2D);

protected:
    bool acceptPMREvent(const View2DMouseEvent * event);
    void pressEvent(const View2DMouseEvent * event);
    void moveEvent(const View2DMouseEvent * event);
    void releaseEvent(const View2DMouseEvent * event);

private:
    // Observed DataObject
    Camera2D * camera2D_;

    // Other member variables
    QPointF cameraPositionAtPress_;
};

#endif // PANVIEW2DACTION_H
