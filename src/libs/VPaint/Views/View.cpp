// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "View.h"

#include "Scene/Scene.h"

View::View(Scene *scene, QWidget * parent) :
    OpenGLWidget(parent),
    scene_(scene)
{
    updateViewOnSceneChange_();
}

Scene * View::scene() const
{
    return scene_;
}

void View::addMouseAction(ViewMouseAction * action)
{
    UniquePtr<ViewMouseAction> ptr(action);
    mouseActions_.push_back(std::move(ptr));
}

void View::mousePressEvent(QMouseEvent *event)
{
    // emit mousePressed(this); XXX (to connect to "MultiView::setActive")

    // Ignore if there is already a mouse event in progress
    if(mouseEvent_)
        return;

    // Generate mouse event
    mouseEvent_ = UniquePtr<ViewMouseEvent>(makeMouseEvent());
    mouseEvent_->setView(this);
    mouseEvent_->setButton(event->button());
    mouseEvent_->setModifiers(event->modifiers());
    mouseEvent_->setViewPosAtPress(event->pos());
    mouseEvent_->setViewPos(event->pos());

    // Select Click action, if any
    mouseClickAction_ = nullptr;
    for (auto & action: mouseActions_)
    {
        if (action->acceptClickEvent(mouseEvent_.get()))
        {
            mouseClickAction_ = action.get();
            break;
        }
    }

    // Select PMR action, if any
    mousePMRAction_ = nullptr;
    for (auto & action: mouseActions_)
    {
        if (action->acceptPMREvent(mouseEvent_.get()))
        {
            mousePMRAction_ = action.get();
            break;
        }
    }

    // Perform action, if any
    if (mouseClickAction_)
    {
        if (mousePMRAction_)
        {
            // Both Click and PMR selected: which of the two to perform will
            // be decided in mouseMoveEvent()
        }
        else
        {
            // Click only selected
            // XXX Shouldn't we perform clic right now?
        }
    }
    else if (mousePMRAction_)
    {
        // PMR only selected: Generate the press event now
        mousePMRAction_->pressEvent(mouseEvent_.get());
    }
    else
    {
        mouseEvent_.release();
        event->ignore();
    }
}

void View::mouseMoveEvent(QMouseEvent *event)
{
    if(mouseEvent_)
    {
        // Set current position
        mouseEvent_->setViewPos(event->pos());

        // Do nothing if Click only
        if (mouseClickAction_ && !mousePMRAction_)
            return;

        // Resolve ambiguity between Click and PMR if possible
        if(mouseClickAction_ && mousePMRAction_)
        {
            QPointF dpos = mouseEvent_->viewPos() -mouseEvent_->viewPosAtPress();
            if (dpos.manhattanLength() > 5.0)
            {
                mouseClickAction_ = nullptr;
                mousePMRAction_->pressEvent(mouseEvent_.get());
                // XXX we should also store all viewPos from move event that were
                // ignored during ambiguity, and call moveEvent on these.
            }
        }

        // Call move event if PMR only
        if(!mouseClickAction_)
        {
            mousePMRAction_->moveEvent(mouseEvent_.get());
        }
    }
    else
    {
        // Move event not part of a PMR event
        // XXX TODO

        /*
        if(!(mouse_LeftButton_ || mouse_MidButton_ || mouse_RightButton_))
        {
            qint64 idleTime = mouse_timerIdleTime_.elapsed();
            if(idleTime>1)
            {
                MoveEvent(mouse_Event_XScene_, mouse_Event_YScene_);
            }
            mouse_timerIdleTime_.start();
            return;
        }
        */
    }

}

void View::mouseReleaseEvent(QMouseEvent *event)
{
    if(mouseEvent_ && mouseEvent_->button() == event->button())
    {
        // Set current position
        mouseEvent_->setViewPos(event->pos());

        // Perform the corresponding actions
        if(mouseClickAction_)
        {
            mouseClickAction_->clickEvent(mouseEvent_.get());
        }
        else if (mousePMRAction_)
        {
            mousePMRAction_->releaseEvent(mouseEvent_.get());
        }

        // Reinitialize
        mouseClickAction_ = nullptr;
        mousePMRAction_ = nullptr;
        mouseEvent_.reset();
    }
}

void View::updateViewOnSceneChange_()
{
    // Note: This is just a regular signal/slot connection, but we need a
    // static_cast here to resolve overload ambiguity, i.e. to tell the compiler
    // to use 'update()' and not, for instance, 'update(const QRect &)'.

    // Type of 'void QWidget::update()'
    using update_t = void (QWidget::*) ();

    // Create connection
    connect(scene_, &Scene::changed, this, static_cast<update_t>(&QWidget::update));
}
