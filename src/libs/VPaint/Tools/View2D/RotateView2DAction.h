// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef ROTATEVIEW2DACTION_H
#define ROTATEVIEW2DACTION_H

#include "Views/View2DMouseAction.h"
#include "Cameras/Camera2DData.h"

#include <QTimer>
#include <QElapsedTimer>

class Camera2D;

class RotateView2DAction: public View2DMouseAction
{
    Q_OBJECT

public:
    RotateView2DAction(Camera2D * camera2D);

protected:
    // PMR = rotate the view
    bool acceptPMREvent(const View2DMouseEvent * event);
    void pressEvent(const View2DMouseEvent * event);
    void moveEvent(const View2DMouseEvent * event);
    void releaseEvent(const View2DMouseEvent * event);

    // Click = reset rotation
    bool acceptClickEvent(const View2DMouseEvent * event);
    void clickEvent(const View2DMouseEvent * event);

private slots:
    void onTimeout_();

private:
    bool accept_(const View2DMouseEvent * event);
    void storeDataAtPress_(const View2DMouseEvent * event);
    void setRotation_(double rotation);
    void completeAnimation_();

private:
    // Observed DataObject
    Camera2D * camera2D_;

    // Other member variables
    Camera2DData cameraDataAtPress_;
    QPointF scenePivot_;
    QPointF viewPivot_;
    QTimer resetAnimationTimer_;
    QElapsedTimer resetAnimationElapsedTimer_;
};

#endif // ROTATEVIEW2DACTION_H
