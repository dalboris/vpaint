// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef SKETCHACTION_H
#define SKETCHACTION_H

#include "Views/View2DMouseAction.h"
#include "OpenVac/Topology/KeyEdge.h"

class Scene;

class SketchAction: public View2DMouseAction
{
public:
    SketchAction(Scene * scene);

protected:
    bool acceptPMREvent(const View2DMouseEvent * event);
    void pressEvent(const View2DMouseEvent * event);
    void moveEvent(const View2DMouseEvent * event);
    void releaseEvent(const View2DMouseEvent * event);

private:
    VGeometry::VCurveInputSample getInputSample_(
            const View2DMouseEvent * event);

    void execTest_();

private:
    Scene * scene_;
    OpenVac::KeyEdgeHandle edge;
};

#endif // SKETCHACTION_H
