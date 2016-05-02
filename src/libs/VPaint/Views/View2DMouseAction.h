// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef VIEW2DMOUSEACTION_H
#define VIEW2DMOUSEACTION_H

#include "Views/ViewMouseAction.h"
#include "Views/View2DMouseEvent.h"

#include <cassert>

class View2DMouseAction: public ViewMouseAction
{
public:
    /// Informs whether or not the action would accept the event as a clic
    /// event. Must be reimplemented by subclasses that may accept clic
    /// events. Default implementation returns \p false.
    virtual bool acceptClickEvent(const View2DMouseEvent * /*event*/)
    {
        return false;
    }

    /// Informs whether or not the action would accept the event as a PMR
    /// event. Must be reimplemented by subclasses that may accept PMR
    /// events. Default implementation returns \p false.
    virtual bool acceptPMREvent(const View2DMouseEvent * /*event*/)
    {
        return false;
    }

    /// Performs the "Click" action.
    virtual void clickEvent(const View2DMouseEvent * /*event*/) {}

    /// Performs the "Press" part of the PMR action.
    virtual void pressEvent(const View2DMouseEvent * /*event*/) {}

    /// Performs the "Move" part of the PMR action.
    virtual void moveEvent(const View2DMouseEvent * /*event*/) {}

    /// Performs the "Release" part of the PMR action.
    virtual void releaseEvent(const View2DMouseEvent * /*event*/) {}

private:
    // Casts from ViewMouseEvent* to View2DMouseEvent*.
    const View2DMouseEvent * cast_(const ViewMouseEvent * event);

    // Cast and forward to protected functions
    bool acceptClickEvent(const ViewMouseEvent * event);
    bool acceptPMREvent(const ViewMouseEvent * event);
    void pressEvent(const ViewMouseEvent * event);
    void moveEvent(const ViewMouseEvent * event);
    void releaseEvent(const ViewMouseEvent * event);
    void clickEvent(const ViewMouseEvent * event);
};

#endif // VIEW2DMOUSEACTION_H
