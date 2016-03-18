// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef VIEW_H
#define VIEW_H

#include "Core/Memory.h"
#include "OpenGL/OpenGLWidget.h"
#include "Views/ViewMouseEvent.h"
#include "Views/ViewMouseAction.h"

#include <vector>

/// \class View
/// \brief A subclass of OpenGLWidget that provides mouse event management.
///
/// This class listens to the low-level Qt mouse events and provides
/// higher-level event handling from then.
/// By design, does not handle multiple clics.
///
class View: public OpenGLWidget
{
public:
    View(QWidget * parent = nullptr);

protected:
    template <class Action>
    void addMouseAction()
    {
        ViewMouseAction * action = new Action();
        UniquePtr<ViewMouseAction> ptr(action);
        mouseActions_.push_back(std::move(ptr));
    }

    virtual ViewMouseEvent * makeMouseEvent() = 0;

protected slots:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    // List of available actions
    std::vector<UniquePtr<ViewMouseAction>> mouseActions_;

    // Clic action and PMR action selected among available actions
    ViewMouseAction * mouseClickAction_ = nullptr;
    ViewMouseAction * mousePMRAction_ = nullptr;

    // Generated mouse event
    UniquePtr<ViewMouseEvent> mouseEvent_;
};

#endif // VIEW_H
