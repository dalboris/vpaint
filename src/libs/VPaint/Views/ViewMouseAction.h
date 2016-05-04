// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef VIEWMOUSEACTION_H
#define VIEWMOUSEACTION_H

#include <QObject>

class ViewMouseEvent;

/// \class ViewMouseAction
/// \brief Base class for mouse-triggered-in-view actions
///
class ViewMouseAction: public QObject
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(ViewMouseAction)

public:
    /// Constructs a ViewMouseAction
    ///
    ViewMouseAction() : QObject(nullptr) {}

    /// Virtual destructor
    ///
    ~ViewMouseAction() {}

protected:
    friend class View;

    /// Informs whether or not the action would accept the event as a clic
    /// event. Must be reimplemented by subclasses that may accept clic
    /// events. Default implementation returns \p false.
    virtual bool acceptClickEvent(const ViewMouseEvent * /*event*/)
    {
        return false;
    }

    /// Informs whether or not the action would accept the event as a PMR
    /// event. Must be reimplemented by subclasses that may accept PMR
    /// events. Default implementation returns \p false.
    virtual bool acceptPMREvent(const ViewMouseEvent * /*event*/)
    {
        return false;
    }

    /// Performs the "Click" action.
    virtual void clickEvent(const ViewMouseEvent * /*event*/) {}

    /// Performs the "Press" part of the PMR action.
    virtual void pressEvent(const ViewMouseEvent * /*event*/) {}

    /// Performs the "Move" part of the PMR action.
    virtual void moveEvent(const ViewMouseEvent * /*event*/) {}

    /// Performs the "Release" part of the PMR action.
    virtual void releaseEvent(const ViewMouseEvent * /*event*/) {}
};

#endif // VIEWMOUSEACTION_H
