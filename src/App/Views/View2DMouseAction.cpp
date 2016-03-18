// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "View2DMouseAction.h"

const View2DMouseEvent * View2DMouseAction::cast_(const ViewMouseEvent * event)
{
    const View2DMouseEvent * res =
            dynamic_cast<const View2DMouseEvent *>(event);
    assert(res);
    return res;
}

bool View2DMouseAction::acceptClickEvent(const ViewMouseEvent * event)
{
    return acceptClickEvent(cast_(event));
}

bool View2DMouseAction::acceptPMREvent(const ViewMouseEvent * event)
{
    return acceptPMREvent(cast_(event));
}

void View2DMouseAction::clickEvent(const ViewMouseEvent * event)
{
    clickEvent(cast_(event));
}

void View2DMouseAction::pressEvent(const ViewMouseEvent * event)
{
    pressEvent(cast_(event));
}

void View2DMouseAction::moveEvent(const ViewMouseEvent * event)
{
    moveEvent(cast_(event));
}

void View2DMouseAction::releaseEvent(const ViewMouseEvent * event)
{
    releaseEvent(cast_(event));
}
