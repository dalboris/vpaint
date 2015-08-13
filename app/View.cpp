// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "View.h"
#include "Scene.h"
#include "Timeline.h"
#include "DevSettings.h"
#include "Global.h"
#include <QtDebug>
#include "OpenGL.h"
#include <QApplication>
#include <QPushButton>
#include <cmath>

#include "VectorAnimationComplex/VAC.h"
#include "VectorAnimationComplex/Cell.h"

// define mouse actions

#define  SELECT_ACTION                                      100
#define  ADDSELECT_ACTION                                   101
#define  DESELECT_ACTION                                    102
#define  TOGGLESELECT_ACTION                                103
#define  DESELECTALL_ACTION                                 104
#define  RECTANGLE_OF_SELECTION_ACTION                      105
#define  DRAG_AND_DROP_ACTION                               106
#define  SPLIT_ACTION                                       107

#define  SKETCH_ACTION                                      200
#define  SKETCH_CHANGE_PEN_WIDTH_ACTION                     203
#define  SKETCH_CHANGE_SNAP_THRESHOLD_ACTION                204
#define  SKETCH_CHANGE_PEN_WIDTH_AND_SNAP_THRESHOLD_ACTION  205

#define  SCULPT_CHANGE_RADIUS_ACTION                        300
#define  SCULPT_DEFORM_ACTION                               301
#define  SCULPT_SMOOTH_ACTION                               302
#define  SCULPT_CHANGE_WIDTH_ACTION                         303

#define  PAINT_ACTION                                       400

View::View(Scene *scene, QWidget *parent) :
    GLWidget(parent, true),
    scene_(scene),
    pickingImg_(0),
    pickingIsEnabled_(true),
    currentAction_(0),
    vac_(0)
{
    // View settings widget
    viewSettingsWidget_ = new ViewSettingsWidget(viewSettings_, this);
    connect(viewSettingsWidget_, SIGNAL(changed()), this, SLOT(update()));
    connect(viewSettingsWidget_, SIGNAL(changed()), this, SIGNAL(settingsChanged()));
    cameraTravellingIsEnabled_ = true;

    // Mac OS X fix: view settings does not appear, so display it in a layout

    connect(this, SIGNAL(viewIsGoingToChange(int, int)), this, SLOT(updatePicking()));
    //connect(this, SIGNAL(viewIsGoingToChange(int, int)), this, SLOT(updateHighlightedObject(int, int)));
    connect(this, SIGNAL(viewIsGoingToChange(int, int)), this, SLOT(update()));

    connect(this, SIGNAL(viewIsBeingChanged(int, int)), this, SLOT(updateZoomFromView()));
    //connect(this, SIGNAL(viewIsBeingChanged(int, int)), this, SLOT(updatePicking()));
    //connect(this, SIGNAL(viewIsBeingChanged(int, int)), this, SLOT(updateHighlightedObject(int, int)));
    connect(this, SIGNAL(viewIsBeingChanged(int, int)), this, SLOT(update()));

    connect(this, SIGNAL(viewChanged(int, int)), this, SLOT(updateZoomFromView()));
    connect(this, SIGNAL(viewChanged(int, int)), this, SLOT(updatePicking()));
    connect(this, SIGNAL(viewChanged(int, int)), this, SLOT(updateHoveredObject(int, int)));
    connect(this, SIGNAL(viewChanged(int, int)), this, SLOT(update()));

    connect(global(),SIGNAL(keyboardModifiersChanged()),this,SLOT(handleNewKeyboardModifiers()));
}

View::~View()
{
    deletePicking();
}

void View::initCamera()
{
    // Set 100% zoom and center canvas in view
    GLWidget_Camera2D camera;
    camera.setZoom(1.0);
    camera.setX(scene()->left() - 0.5*(scene()->width() - width()));
    camera.setY(scene()->top() - 0.5*(scene()->height() - height()));
    setCamera2D(camera);
}

Scene * View::scene()
{
    return scene_;
}

void View::resizeEvent(QResizeEvent * event)
{
    if(autoCenterScene_)
        initCamera();

    GLWidget::resizeEvent(event);
}

void View::resizeGL(int width, int height)
{
    GLWidget::resizeGL(width, height);
    updatePicking();
}

void View::keyPressEvent(QKeyEvent *event)
{
    event->ignore();
}


void View::keyReleaseEvent(QKeyEvent *event)
{
    event->ignore();
}

void View::handleNewKeyboardModifiers()
{
    vac_ = scene_->vectorAnimationComplex();

    // Rectangle of selection
    if(vac_ && currentAction_ == RECTANGLE_OF_SELECTION_ACTION)
    {
        vac_->setSelectedCellsFromRectangleOfSelection(global()->keyboardModifiers());
    }

    // Update in any case, better be safe.
    emit allViewsNeedToUpdate();
}

MouseEvent View::mouseEvent() const
{
    MouseEvent me;
    me.x = mouse_Event_XScene_;
    me.y = mouse_Event_YScene_;
    me.left = mouse_LeftButton_;
    me.mid = mouse_MidButton_;
    me.right = mouse_RightButton_;
    me.alt = mouse_AltWasDown_;
    me.control = mouse_ControlWasDown_;
    me.shift = mouse_ShiftWasDown_;
    return me;
}

void View::update()
{
    GLWidget_Camera2D c = camera2D();
    c.setZoom(viewSettings_.zoom());
    setCamera2D(c);
    updateGL();
}

void View::updateZoomFromView()
{
    viewSettings_.setZoom(zoom());
    viewSettingsWidget_->updateWidgetFromSettings();
    viewSettingsWidget_->updateSettingsFromWidgetSilent();
    GLWidget_Camera2D c = camera2D();
    c.setZoom(viewSettings_.zoom());
    setCamera2D(c);
}

int View::decideClicAction()
{
    // Selection
    if( (global()->toolMode() == Global::SELECT && mouse_LeftButton_) //||
        //(global()->toolMode() == Global::SKETCH && mouse_LeftButton_) ||
        //(global()->toolMode() == Global::SCULPT && mouse_LeftButton_) ||
        //(global()->toolMode() == Global::CUT && mouse_LeftButton_) ||
        //(mouse_RightButton_)
        )
    {
        // Left = set selection
        if(!mouse_AltWasDown_ &&
           !mouse_ControlWasDown_ &&
           !mouse_ShiftWasDown_)
        {
            if(hoveredObject_.isNull())
                return DESELECTALL_ACTION;
            else
                return SELECT_ACTION;
        }
        // Shift + Left = add to selection
        if(!mouse_AltWasDown_ &&
           !mouse_ControlWasDown_ &&
           mouse_ShiftWasDown_)
        {
            return ADDSELECT_ACTION;
        }
        // Alt + Left = remove from selection
        if(mouse_AltWasDown_ &&
           !mouse_ControlWasDown_ &&
           !mouse_ShiftWasDown_)
        {
            return DESELECT_ACTION;
        }
        // Alt + Shift + Left = toggle selection state
        if(mouse_AltWasDown_ &&
           !mouse_ControlWasDown_ &&
           mouse_ShiftWasDown_)
        {
            return TOGGLESELECT_ACTION;
        }
    }

    // Cut edge
    if( global()->toolMode() == Global::SELECT &&
         mouse_LeftButton_ &&
         !mouse_AltWasDown_ &&
         mouse_ControlWasDown_ &&
         !mouse_ShiftWasDown_)
    {
        return SPLIT_ACTION;
    }

    // Paint
    if( global()->toolMode() == Global::PAINT &&
        mouse_LeftButton_ &&
        !mouse_AltWasDown_ &&
        !mouse_ControlWasDown_ &&
        !mouse_ShiftWasDown_)
    {
        return PAINT_ACTION;
    }

    return GLWidget::decideClicAction();
}

int View::decidePMRAction()
{
    vac_ = scene_->vectorAnimationComplex();

    if(global()->toolMode() == Global::SELECT)
    {
        if( !hoveredObject_.isNull() &&
            mouse_LeftButton_ &&
            !mouse_AltWasDown_ &&
            !mouse_ControlWasDown_ &&
            !mouse_ShiftWasDown_ &&
            vac_)
        {
            return DRAG_AND_DROP_ACTION;
        }
        else if (hoveredObject_.isNull() &&
                 mouse_LeftButton_ &&
                 !mouse_ControlWasDown_ )
        {
            return RECTANGLE_OF_SELECTION_ACTION;
        }
    }
    else if(global()->toolMode() == Global::SKETCH)
    {
        if(mouse_LeftButton_ &&
           !mouse_AltWasDown_ &&
           !mouse_ControlWasDown_ &&
           !mouse_ShiftWasDown_ &&
           vac_)
        {
            return SKETCH_ACTION;
        }
        // Ctrl + Left
        if(mouse_LeftButton_ &&
           !mouse_AltWasDown_ &&
           mouse_ControlWasDown_ &&
           !mouse_ShiftWasDown_ &&
           vac_)
        {
            return SKETCH_CHANGE_PEN_WIDTH_ACTION;
        }
        // Alt + Left
        if(mouse_LeftButton_ &&
           mouse_AltWasDown_ &&
           !mouse_ControlWasDown_ &&
           !mouse_ShiftWasDown_ &&
           vac_)
        {
            return SKETCH_CHANGE_SNAP_THRESHOLD_ACTION;
        }
        // Ctrl + Alt + Left
        if(mouse_LeftButton_ &&
           mouse_AltWasDown_ &&
           mouse_ControlWasDown_ &&
           !mouse_ShiftWasDown_ &&
           vac_)
        {
            return SKETCH_CHANGE_PEN_WIDTH_AND_SNAP_THRESHOLD_ACTION;
        }
    }
    else if(global()->toolMode() == Global::SCULPT)
    {
        if(mouse_LeftButton_ &&
           !mouse_AltWasDown_ &&
           !mouse_ControlWasDown_ &&
           !mouse_ShiftWasDown_ &&
           vac_)
        {
            VectorAnimationComplex::Cell * hoveredCell =
                        vac_->hoveredCell();

            if(hoveredCell && hoveredCell->toVertexCell())
            {
                return DRAG_AND_DROP_ACTION;
            }
            else
            {
                return SCULPT_DEFORM_ACTION;
            }
        }
        // Ctrl + Left
        if(mouse_LeftButton_ &&
           !mouse_AltWasDown_ &&
           mouse_ControlWasDown_ &&
           !mouse_ShiftWasDown_ &&
           vac_)
        {
            return SCULPT_CHANGE_RADIUS_ACTION;
        }
        // Alt + Left
        if(mouse_LeftButton_ &&
           mouse_AltWasDown_ &&
           !mouse_ControlWasDown_ &&
           !mouse_ShiftWasDown_ &&
           vac_)
        {
            return SCULPT_CHANGE_WIDTH_ACTION;
        }
        // Shift + Left
        if(mouse_LeftButton_ &&
           !mouse_AltWasDown_ &&
           !mouse_ControlWasDown_ &&
           mouse_ShiftWasDown_ &&
           vac_)
        {
            return SCULPT_SMOOTH_ACTION;
        }
    }

    return GLWidget::decidePMRAction();
}

Time View::activeTime() const
{
    return viewSettings_.time();
}

void View::setActiveTime(Time t)
{
    viewSettings_.setTime(t);
    viewSettingsWidget_->updateWidgetFromSettings();
}

void View::setActive(bool isActive)
{
    viewSettingsWidget_->setActive(isActive);
}

void View::ClicEvent(int action, double x, double y)
{
    // It is View's responsibility to call update() or updatePicking()

    if(action==SPLIT_ACTION)
    {
        if(!hoveredObject_.isNull() || global()->toolMode() == Global::SKETCH)
        {
            vac_ = scene_->vectorAnimationComplex();
            if(vac_)
            {
                vac_->split(x, y, interactiveTime(), true);

                emit allViewsNeedToUpdatePicking();
                updateHoveredObject(mouse_Event_X_, mouse_Event_Y_);
                emit allViewsNeedToUpdate();
            }
        }
    }
    else if(action==PAINT_ACTION)
    {
        vac_ = scene_->vectorAnimationComplex();
        if(vac_)
        {
            vac_->paint(x, y, interactiveTime());
            emit allViewsNeedToUpdatePicking();
            updateHoveredObject(mouse_Event_X_, mouse_Event_Y_);
            emit allViewsNeedToUpdate();
        }
    }

    else if(action==SELECT_ACTION)
    {
        if(!hoveredObject_.isNull())
        {
            scene_->deselectAll();
            scene_->select(activeTime(),
                           hoveredObject_.index(),
                           hoveredObject_.id());
            emit allViewsNeedToUpdate();
        }
    }
    else if(action==DESELECTALL_ACTION)
    {
        scene_->deselectAll();
        emit allViewsNeedToUpdate();
    }
    else if(action==ADDSELECT_ACTION)
    {
        if(!hoveredObject_.isNull())
        {
            scene_->select(activeTime(),
                           hoveredObject_.index(),
                           hoveredObject_.id());
            emit allViewsNeedToUpdate();
        }
    }
    else if(action==DESELECT_ACTION)
    {
        if(!hoveredObject_.isNull())
        {
            scene_->deselect(activeTime(),
                             hoveredObject_.index(),
                             hoveredObject_.id());
            emit allViewsNeedToUpdate();
        }
    }
    else if(action==TOGGLESELECT_ACTION)
    {
        if(!hoveredObject_.isNull())
        {
            scene_->toggle(activeTime(),
                           hoveredObject_.index(),
                           hoveredObject_.id());
            emit allViewsNeedToUpdate();
        }
    }
    else
    {
        GLWidget::ClicEvent(action, x, y);
    }
}

void View::MoveEvent(double x, double y)
{
    // Boolean deciding if the scene must be redrawn even though only the mouse
    // has moved with no action performed. This is possible because depending on
    // where the mouse is, the action to-be-performed can be different, and
    // feedback to user on what is action would be must be given to user before
    // the action is undertaken
    bool mustRedraw = false;
    global()->setSceneCursorPos(Eigen::Vector2d(x,y));

    // Update highlighted object
    bool hoveredObjectChanged = updateHoveredObject(mouse_Event_X_, mouse_Event_Y_);
    if(hoveredObjectChanged)
        mustRedraw = true;

    // Update to-be-drawn straight line
    Qt::KeyboardModifiers keys = global()->keyboardModifiers();
    if( (global()->toolMode() == Global::SKETCH) )
    {
        if(keys & Qt::ControlModifier)
        {
            //scene_->updateCellsToConsiderForCutting();
            mustRedraw = true;
        }
        else
        {
            //scene_->resetCellsToConsiderForCutting();
            // Must be redrawn anyway to redraw the cursor
            mustRedraw = true;
        }
    }

    // Update to-be-sculpted edge
    if(global()->toolMode() == Global::SCULPT)
    {
        Time time = interactiveTime();
        scene_->vectorAnimationComplex()->updateSculpt(x, y, time);
        mustRedraw = true;
    }

    // Update to-be-painted face
    if(global()->toolMode() == Global::PAINT)
    {
        Time time = interactiveTime();
        scene_->vectorAnimationComplex()->updateToBePaintedFace(x, y, time);
        mustRedraw = true;
    }

    // Redraw if necessary
    if(mustRedraw)
    {
        // so that the highlighted object is also highlighted in other views
        // this is a matter of preference, we could call only "update()" if
        // we don't like this behaviour. But I like it, personally. Maybe
        // I could add it as a user preference
        emit allViewsNeedToUpdate();
    }
}

Time View::interactiveTime() const
{
    return viewSettings_.time();
}


void View::PMRPressEvent(int action, double x, double y)
{
    currentAction_ = action;

    // It is View's responsibility to call update() or updatePicking
    // for mouse PMR actions
    global()->setSceneCursorPos(Eigen::Vector2d(x,y));

    if(action==SKETCH_ACTION)
    {
        // here, possibly,  the scene  has several layers  that it
        // knows about, as  well as which one is  active, and then
        // returns the active one.
        //
        // but the  scene doesn't know  at which time the  user is
        // drawing, since it depends  on the view.  (should active
        // layer depends on the view?  -> my current answer is no,
        // too  confusing.   But could  be  an option,  eventually
        // disable by default.  It would increases the flexibility
        // of the software).
        //
        // Current approach is then:
        //   1) The scene only knows which layer (ASG) is active
        //   2) The view only knows the time we are drawing in

        // Future ideas:
        //   Each  view  would  be  able  to see  the  scene  with
        //   different  translation/scale/rotation  (eg each  view
        //   has its own camera). Hence here, first the point
        //   (int xView, int yView) is converted into the point
        //   pos = (double xScene, double yScene)


        lastMousePos_ = QPoint(mouse_Event_X_,mouse_Event_Y_);

        // pos = viewToScene(x,y); <- by now: the identity
        QPointF pos = QPointF(x,y);
        double xScene = pos.rx();
        double yScene = pos.ry();

        double w = global()->settings().edgeWidth();
        bool debug = false;
        if(!debug)
        {
            if(mouse_isTablet_ &&  global()->useTabletPressure())
                w *= 2 * mouse_tabletPressure_; // 2 so that a half-pressure would get the default width
        }
        vac_->beginSketchEdge(xScene,yScene, w, interactiveTime());

        //emit allViewsNeedToUpdatePicking();
        //updateHighlightedObject(mouse_Event_X_, mouse_Event_Y_);
        emit allViewsNeedToUpdate();
    }
    else if(action==DRAG_AND_DROP_ACTION)
    {
        vac_->prepareDragAndDrop(mouse_PressEvent_XScene_, mouse_PressEvent_YScene_, interactiveTime());
    }
    else if(action==RECTANGLE_OF_SELECTION_ACTION)
    {
        vac_->beginRectangleOfSelection(x,y,interactiveTime());
    }
    else if(action==SCULPT_CHANGE_RADIUS_ACTION)
    {
        sculptStartRadius_ = global()->sculptRadius();
        sculptStartX_ = x;
        sculptStartY_ = y;
        sculptRadiusDx_ = 0;
        sculptRadiusDy_ = 0;

        //emit allViewsNeedToUpdatePicking();
        //updateHighlightedObject(mouse_Event_X_, mouse_Event_Y_);
        //emit allViewsNeedToUpdate();
    }
    else if(action==SKETCH_CHANGE_PEN_WIDTH_ACTION)
    {
        sculptStartRadius_ = global()->edgeWidth();
        sculptStartX_ = x;
        sculptStartY_ = y;
        sculptRadiusDx_ = 0;
        sculptRadiusDy_ = 0;

        //emit allViewsNeedToUpdatePicking();
        //updateHighlightedObject(mouse_Event_X_, mouse_Event_Y_);
        //emit allViewsNeedToUpdate();
    }
    else if(action==SKETCH_CHANGE_SNAP_THRESHOLD_ACTION)
    {
        sculptStartRadius_ = global()->snapThreshold();
        sculptStartX_ = x;
        sculptStartY_ = y;
        sculptRadiusDx_ = 0;
        sculptRadiusDy_ = 0;

        //emit allViewsNeedToUpdatePicking();
        //updateHighlightedObject(mouse_Event_X_, mouse_Event_Y_);
        //emit allViewsNeedToUpdate();
    }
    else if(action==SKETCH_CHANGE_PEN_WIDTH_AND_SNAP_THRESHOLD_ACTION)
    {
        sculptStartRadius_ = global()->edgeWidth();
        sculptStartRadius2_ = global()->snapThreshold();
        sculptStartX_ = x;
        sculptStartY_ = y;
        sculptRadiusDx_ = 0;
        sculptRadiusDy_ = 0;

        //emit allViewsNeedToUpdatePicking();
        //updateHighlightedObject(mouse_Event_X_, mouse_Event_Y_);
        //emit allViewsNeedToUpdate();
    }
    else if(action==SCULPT_DEFORM_ACTION)
    {
        sculptStartRadius_ = global()->sculptRadius();
        sculptStartX_ = x;
        sculptStartY_ = y;
        vac_->beginSculptDeform(x,y);

        //emit allViewsNeedToUpdatePicking();
        //updateHighlightedObject(mouse_Event_X_, mouse_Event_Y_);
        //emit allViewsNeedToUpdate();
    }
    else if(action==SCULPT_CHANGE_WIDTH_ACTION)
    {
        sculptStartRadius_ = global()->sculptRadius();
        sculptStartX_ = x;
        sculptStartY_ = y;
        vac_->beginSculptEdgeWidth(x,y);

        //emit allViewsNeedToUpdatePicking();
        //updateHighlightedObject(mouse_Event_X_, mouse_Event_Y_);
        //emit allViewsNeedToUpdate();
    }
    else if(action==SCULPT_SMOOTH_ACTION)
    {
        sculptStartRadius_ = global()->sculptRadius();
        sculptStartX_ = x;
        sculptStartY_ = y;
        vac_->beginSculptSmooth(x,y);

        //emit allViewsNeedToUpdatePicking();
        //updateHighlightedObject(mouse_Event_X_, mouse_Event_Y_);
        //emit allViewsNeedToUpdate();
    }
    else
        GLWidget::PMRPressEvent(action, x, y);
}

void View::PMRMoveEvent(int action, double x, double y)
{
    global()->setSceneCursorPos(Eigen::Vector2d(x,y));

    if(action==SKETCH_ACTION)
    {
        QPoint mousePos = QPoint(mouse_Event_X_,mouse_Event_Y_);

        if(lastMousePos_ != mousePos && vac_)
        {
            lastMousePos_ = mousePos;

            double w = global()->settings().edgeWidth();
            bool debug = false;
            if(!debug)
            {
                if(mouse_isTablet_ &&  global()->useTabletPressure())
                    w *= 2 * mouse_tabletPressure_; // 2 so that a half-pressure would get the default width
            }
            vac_->continueSketchEdge(x,y, w); // Note: this call "changed", hence all views are updated
        }

        //emit allViewsNeedToUpdatePicking();
        updateHoveredObject(mouse_Event_X_, mouse_Event_Y_);
        emit allViewsNeedToUpdate();
    }
    else if(action==DRAG_AND_DROP_ACTION)
    {
        vac_->performDragAndDrop(x, y);

        //emit allViewsNeedToUpdatePicking();
        //updateHighlightedObject(mouse_Event_X_, mouse_Event_Y_);
        emit allViewsNeedToUpdate();

    }
    else if(action==RECTANGLE_OF_SELECTION_ACTION)
    {
        vac_->continueRectangleOfSelection(x,y);

        emit allViewsNeedToUpdate();
    }
    else if(action==SCULPT_CHANGE_RADIUS_ACTION)
    {
        // Increment how much we moved
        // method hiding the cursor and let it at the same position as on press
        // obviously can't work with pen tablets, since position is absolute...
        //sculptRadiusDx_ += x - sculptStartX_;
        //sculptRadiusDy_ += y - sculptStartY_;
        // hence just use the plain vanilla method
        sculptRadiusDx_ = x - sculptStartX_;
        sculptRadiusDy_ = y - sculptStartY_; // yes, this is useless, but can be useful later

        // Put mouse position back from where it was
        //QPoint p(mapToGlobal(QPoint(sculptStartX_,sculptStartY_)));
        //QCursor::setPos(p);

        // update radius
        double newRadius = sculptStartRadius_ + sculptRadiusDx_;
        if(newRadius < 0)
            newRadius *= -1;
        global()->setSculptRadius(newRadius);

        //emit allViewsNeedToUpdatePicking();
        //updateHighlightedObject(mouse_Event_X_, mouse_Event_Y_);
        emit allViewsNeedToUpdate();

    }
    else if(action==SKETCH_CHANGE_PEN_WIDTH_ACTION)
    {
        // Get delta
        sculptRadiusDx_ = x - sculptStartX_;
        sculptRadiusDy_ = y - sculptStartY_;

        // Get new radius
        double newRadius = sculptStartRadius_ + sculptRadiusDx_;
        if(newRadius < 0)
            newRadius *= -1;
        global()->setEdgeWidth(newRadius);

        // Prevent painted cursor gadget to move
        global()->setSceneCursorPos(Eigen::Vector2d(mouse_PressEvent_XScene_, mouse_PressEvent_YScene_));

        // Update
        emit allViewsNeedToUpdate();
    }
    else if(action==SKETCH_CHANGE_SNAP_THRESHOLD_ACTION)
    {
        // Get delta
        sculptRadiusDx_ = x - sculptStartX_;
        sculptRadiusDy_ = y - sculptStartY_;

        // Get new radius
        double newRadius = sculptStartRadius_ + sculptRadiusDx_;
        if(newRadius < 0)
            newRadius *= -1;
        global()->setSnapThreshold(newRadius);

        // Prevent painted cursor gadget to move
        global()->setSceneCursorPos(Eigen::Vector2d(mouse_PressEvent_XScene_, mouse_PressEvent_YScene_));

        // Update
        emit allViewsNeedToUpdate();
    }
    else if(action==SKETCH_CHANGE_PEN_WIDTH_AND_SNAP_THRESHOLD_ACTION)
    {
        // Get delta
        sculptRadiusDx_ = x - sculptStartX_;
        sculptRadiusDy_ = y - sculptStartY_;

        // Get new radius
        double newRadius = sculptStartRadius_ + sculptRadiusDx_;
        if(newRadius < 0)
            newRadius *= -1;
        global()->setEdgeWidth(newRadius);

        // Get new radius 2
        double newRadius2 = 0;
        if(sculptStartRadius_ > 0)
        {
            newRadius2 = sculptStartRadius2_ * newRadius / sculptStartRadius_;
        }
        else
        {
            newRadius2 = sculptStartRadius2_ + sculptRadiusDx_;
        }
        if(newRadius2 < 0)
            newRadius2 *= -1;
        global()->setSnapThreshold(newRadius2);

       // Prevent painted cursor gadget to move
        global()->setSceneCursorPos(Eigen::Vector2d(mouse_PressEvent_XScene_, mouse_PressEvent_YScene_));

        // Update
        emit allViewsNeedToUpdate();
    }
    else if(action==SCULPT_DEFORM_ACTION)
    {
        vac_->continueSculptDeform(x,y);

        //emit allViewsNeedToUpdatePicking();
        //updateHighlightedObject(mouse_Event_X_, mouse_Event_Y_);
        emit allViewsNeedToUpdate();
    }
    else if(action==SCULPT_CHANGE_WIDTH_ACTION)
    {
        vac_->continueSculptEdgeWidth(x,y);

        //emit allViewsNeedToUpdatePicking();
        //updateHighlightedObject(mouse_Event_X_, mouse_Event_Y_);
        emit allViewsNeedToUpdate();
    }
    else if(action==SCULPT_SMOOTH_ACTION)
    {
        vac_->continueSculptSmooth(x,y);

        //emit allViewsNeedToUpdatePicking();
        //updateHighlightedObject(mouse_Event_X_, mouse_Event_Y_);
        emit allViewsNeedToUpdate();
    }
    else
        GLWidget::PMRMoveEvent(action, x, y);
}
void View::PMRReleaseEvent(int action, double x, double y)
{
    currentAction_ = 0;

    global()->setSceneCursorPos(Eigen::Vector2d(x,y));

    if(action==SKETCH_ACTION)
    {
        vac_->endSketchEdge();

        emit allViewsNeedToUpdatePicking();
        updateHoveredObject(mouse_Event_X_, mouse_Event_Y_);
        emit allViewsNeedToUpdate();
    }
    else if(action==DRAG_AND_DROP_ACTION)
    {
        vac_->completeDragAndDrop();

        emit allViewsNeedToUpdatePicking();
        updateHoveredObject(mouse_Event_X_, mouse_Event_Y_);
        emit allViewsNeedToUpdate();
    }
    else if(action==RECTANGLE_OF_SELECTION_ACTION)
    {
        vac_->endRectangleOfSelection();

        updateHoveredObject(mouse_Event_X_, mouse_Event_Y_);
        emit allViewsNeedToUpdate();
    }
    else if(action==SCULPT_CHANGE_RADIUS_ACTION)
    {
        vac_->updateSculpt(x, y, interactiveTime());

        emit allViewsNeedToUpdatePicking();
        updateHoveredObject(mouse_Event_X_, mouse_Event_Y_);
        emit allViewsNeedToUpdate();
    }
    else if(action==SKETCH_CHANGE_PEN_WIDTH_AND_SNAP_THRESHOLD_ACTION)
    {
        emit allViewsNeedToUpdatePicking();
        updateHoveredObject(mouse_Event_X_, mouse_Event_Y_);
        emit allViewsNeedToUpdate();
    }
    else if(action==SCULPT_DEFORM_ACTION)
    {
        vac_->endSculptDeform();
        vac_->updateSculpt(x, y, interactiveTime());

        emit allViewsNeedToUpdatePicking();
        updateHoveredObject(mouse_Event_X_, mouse_Event_Y_);
        emit allViewsNeedToUpdate();
    }
    else if(action==SCULPT_CHANGE_WIDTH_ACTION)
    {
        vac_->endSculptEdgeWidth();
        vac_->updateSculpt(x, y, interactiveTime());

        emit allViewsNeedToUpdatePicking();
        updateHoveredObject(mouse_Event_X_, mouse_Event_Y_);
        emit allViewsNeedToUpdate();
    }
    else if(action==SCULPT_SMOOTH_ACTION)
    {
        vac_->endSculptSmooth();
        vac_->updateSculpt(x, y, interactiveTime());

        emit allViewsNeedToUpdatePicking();
        updateHoveredObject(mouse_Event_X_, mouse_Event_Y_);
        emit allViewsNeedToUpdate();
    }
    else
        GLWidget::PMRReleaseEvent(action, x, y);

}

/***********************************************************
 *              DRAWING
 */


void View::drawScene()
{
    if(!mouse_HideCursor_)
    {
        switch(global()->toolMode())
        {
        case Global::SELECT:
            setCursor(Qt::ArrowCursor);
            break;
        case Global::SKETCH:
            setCursor(Qt::CrossCursor);
            break;
        case Global::PAINT:
            setCursor(Qt::CrossCursor);
            break;
        case Global::SCULPT:
            setCursor(Qt::CrossCursor);
            break;
        default:
            break;
        }
    }

    glClearColor(1.0,1.0,1.0,1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    scene_->drawCanvas(viewSettings_);

    viewSettings_.setDrawBackground(true);
    viewSettings_.setMainDrawing(false);

    Time t = activeTime();
    {
        if(viewSettings_.onionSkinningIsEnabled())
        {
            Time tOnion = t;
            for(int i=0; i<viewSettings_.numOnionSkinsBefore(); ++i)
            {
                glTranslated(-viewSettings_.onionSkinsXOffset(),-viewSettings_.onionSkinsYOffset(),0);
                tOnion = tOnion - viewSettings_.onionSkinsTimeOffset();
                scene_->draw(tOnion, viewSettings_);
                viewSettings_.setDrawBackground(false);
            }
            for(int i=0; i<viewSettings_.numOnionSkinsBefore(); ++i)
            {
                glTranslated(viewSettings_.onionSkinsXOffset(),viewSettings_.onionSkinsYOffset(),0);
            }

            tOnion = t;
            for(int i=0; i<viewSettings_.numOnionSkinsAfter(); ++i)
            {
                glTranslated(viewSettings_.onionSkinsXOffset(),viewSettings_.onionSkinsYOffset(),0);
                tOnion = tOnion + viewSettings_.onionSkinsTimeOffset();
                scene_->draw(tOnion, viewSettings_);
                viewSettings_.setDrawBackground(false);
            }
            for(int i=0; i<viewSettings_.numOnionSkinsAfter(); ++i)
            {
                glTranslated(-viewSettings_.onionSkinsXOffset(),-viewSettings_.onionSkinsYOffset(),0);
            }
        }

        // Draw current frame
        viewSettings_.setMainDrawing(true);
        scene_->draw(t, viewSettings_);
    }
}

void View::toggleOutline()
{
    viewSettings_.toggleOutline();
    viewSettingsWidget_->updateWidgetFromSettings();
    update();
}

void View::toggleOutlineOnly()
{
    viewSettings_.toggleOutlineOnly();
    viewSettingsWidget_->updateWidgetFromSettings();
    update();
}

void View::setDisplayMode(ViewSettings::DisplayMode displayMode)
{
    viewSettings_.setDisplayMode(displayMode);
    viewSettingsWidget_->updateWidgetFromSettings();
    update();
}

void View::setOnionSkinningEnabled(bool enabled)
{
    viewSettings_.setOnionSkinningIsEnabled(enabled);
    viewSettingsWidget_->updateWidgetFromSettings();
    update();
}

void View::fitAllInWindow()
{
    // TODO
}

void View::fitSelectionInWindow()
{
    // TODO
}

double View::zoom() const
{
    return camera2D().zoom();
}

double  View::xSceneMin() const
{
    return - camera2D().x() / zoom();
}

double  View::ySceneMin() const
{
    return - camera2D().y() / zoom();
}

double  View::xSceneMax() const
{
    double x = xSceneMin();
    double w = width();
    double z = zoom();

    return x+w/z;
}

double  View::ySceneMax() const
{
    double x = ySceneMin();
    double w = height();
    double z = zoom();

    return x+w/z;
}

ViewSettings View::viewSettings() const
{
    return viewSettings_;
}

ViewSettingsWidget * View::viewSettingsWidget() const
{
    return viewSettingsWidget_;
}


/***********************************************************
 *              PICKING
 */

void View::drawPick()
{
    Time t = activeTime();
    {
        if(viewSettings_.onionSkinningIsEnabled() && viewSettings_.areOnionSkinsPickable())
        {
            Time tOnion = t;
            for(int i=0; i<viewSettings_.numOnionSkinsBefore(); ++i)
            {
                glTranslated(-viewSettings_.onionSkinsXOffset(),-viewSettings_.onionSkinsYOffset(),0);
                tOnion = tOnion - viewSettings_.onionSkinsTimeOffset();
                scene_->drawPick(tOnion, viewSettings_);
            }
            for(int i=0; i<viewSettings_.numOnionSkinsBefore(); ++i)
            {
                glTranslated(viewSettings_.onionSkinsXOffset(),viewSettings_.onionSkinsYOffset(),0);
            }

            tOnion = t;
            for(int i=0; i<viewSettings_.numOnionSkinsAfter(); ++i)
            {
                glTranslated(viewSettings_.onionSkinsXOffset(),viewSettings_.onionSkinsYOffset(),0);
                tOnion = tOnion + viewSettings_.onionSkinsTimeOffset();
                scene_->drawPick(tOnion, viewSettings_);
            }
            for(int i=0; i<viewSettings_.numOnionSkinsAfter(); ++i)
            {
                glTranslated(-viewSettings_.onionSkinsXOffset(),-viewSettings_.onionSkinsYOffset(),0);
            }
        }

        // Draw current frame
        scene_->drawPick(t, viewSettings_);
    }
}

bool View::updateHoveredObject(int x, int y)
{
    // make sure this does NOT redraw the scene, just change its highlighted status.

    if(!pickingIsEnabled_)
        return false;

    // Don't do anything if no picking image
    if(!pickingImg_)
        return false;

    // Find object under the mouse
    Picking::Object old = hoveredObject_;
    if(x<0 || (uint)x>=WINDOW_SIZE_X_ || y<0 || (uint)y>=WINDOW_SIZE_Y_)
    {
        hoveredObject_ = Picking::Object();
    }
    else
    {
        hoveredObject_ = getCloserObject(x, y);
    }

    // Check if it has changed
    bool hasChanged = !(hoveredObject_ == old);

    // If it has, inform the scene of the new highlighted state
    if(hasChanged)
    {
        if(hoveredObject_.isNull())
            scene_->setNoHoveredObject();
        else
            scene_->setHoveredObject(
                activeTime(),
                hoveredObject_.index(),
                hoveredObject_.id());
    }
    else
    {
        // even if it has not changed, inform the scene when nothing's highlighted
        if(hoveredObject_.isNull())
            scene_->setNoHoveredObject();
    }

    return hasChanged;
}

uchar * View::pickingImg(int x, int y)
{
    int k = 4*( (WINDOW_SIZE_Y_ - y - 1)*WINDOW_SIZE_X_ + x);
    return &pickingImg_[k];
}

// This method must be very fast. Assumes x and y in range
Picking::Object View::getCloserObject(int x, int y)
{
    // First look directly whether there's an object right at mouse position
    uchar * p = pickingImg(x,y);
    uchar r=p[0], g=p[1], b=p[2];
    if(r!=255 || g!=255 || b!=255)
    {
        return Picking::objectFromRGB(r,g,b);
    }
    else
    {
        // If not, look around in a radius of D pixels
        int D = 3;

        // Clipping
        if(x<D)
            D = x;
        if(y<D)
            D = y;
        int rightBorderDist = WINDOW_SIZE_X_-1-x;
        if(rightBorderDist<D)
            D = rightBorderDist;
        int bottomBorderDist = WINDOW_SIZE_Y_-1-y;
        if(bottomBorderDist<D)
            D = bottomBorderDist;


        for(int d=1; d<=D; d++)
        {
            // top row
            for(int varX=x-d; varX<=x+d; varX++)
            {
                p = pickingImg(varX,y-d);
                r=p[0], g=p[1], b=p[2];
                if(r!=255 || g!=255 || b!=255)
                    return Picking::objectFromRGB(r,g,b);
            }
            // bottom row
            for(int varX=x-d; varX<=x+d; varX++)
            {
                p = pickingImg(varX,y+d);
                r=p[0], g=p[1], b=p[2];
                if(r!=255 || g!=255 || b!=255)
                    return Picking::objectFromRGB(r,g,b);
            }
            // left column
            for(int varY=y-d; varY<=y+d; varY++)
            {
                p = pickingImg(x-d,varY);
                r=p[0], g=p[1], b=p[2];
                if(r!=255 || g!=255 || b!=255)
                    return Picking::objectFromRGB(r,g,b);
            }
            // right column
            for(int varY=y-d; varY<=y+d; varY++)
            {
                p = pickingImg(x+d,varY);
                r=p[0], g=p[1], b=p[2];
                if(r!=255 || g!=255 || b!=255)
                    return Picking::objectFromRGB(r,g,b);
            }
        }

        // If still no object found, return a null object
        return Picking::Object();
    }
}

void View::deletePicking()
{
    if(pickingImg_)
    {
        glDeleteFramebuffers(1, &fboId_);
        glDeleteRenderbuffers(1, &rboId_);
        glDeleteTextures(1, &textureId_);
        hoveredObject_ = Picking::Object();
        delete[] pickingImg_;
        pickingImg_ = 0;
        WINDOW_SIZE_X_ = 0;
        WINDOW_SIZE_Y_ = 0;
    }
}

void View::newPicking()
{
    //  code adapted from http://www.songho.ca/opengl/gl_fbo.html

    // create a texture object
    glGenTextures(1, &textureId_);
    glBindTexture(GL_TEXTURE_2D, textureId_);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, WINDOW_SIZE_X_, WINDOW_SIZE_Y_, 0,
             GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // test availability of OpenGL version
    if(GLEW_VERSION_2_0)
    {
        //qDebug("supported");
    }
    else
    {
        qDebug("not supported");
    }

    if(glewIsSupported("GL_ARB_framebuffer_object"))
    {
        //qDebug("fbo supported");
    }
    else
    {
        qDebug("fbo not supported");
    }

    // create a renderbuffer object to store depth info
    glGenRenderbuffers(1, &rboId_);
    glBindRenderbuffer(GL_RENDERBUFFER, rboId_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
                    WINDOW_SIZE_X_, WINDOW_SIZE_Y_);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // create a framebuffer object
    glGenFramebuffers(1, &fboId_);
    glBindFramebuffer(GL_FRAMEBUFFER, fboId_);

    // attach the texture to FBO color attachment point
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                     GL_TEXTURE_2D, textureId_, 0);

    // attach the renderbuffer to depth attachment point
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                      GL_RENDERBUFFER, rboId_);

    // check FBO status
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
    {
        qDebug() << "ERROR void View::newPicking()"
               << "FBO status != GL_FRAMEBUFFER_COMPLETE";
        return;
    }

    // switch back to window-system-provided framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // allocate memory for picking
    pickingImg_ = new uchar[4 * WINDOW_SIZE_X_ * WINDOW_SIZE_Y_];
}

#include <QElapsedTimer>
#include <QtDebug>

void View::enablePicking()
{
    pickingIsEnabled_ = true;
}

void View::disablePicking()
{
    pickingIsEnabled_ = false;
}

namespace
{

void imageCleanupHandler(void * info)
{
    uchar * img = reinterpret_cast<uchar*>(info);
    delete[] img;
}

}

QImage View::drawToImage(double x, double y, double w, double h, int imgW, int imgH, bool transparentBackground)
{
    return drawToImage(activeTime(), x, y, w, h, imgW, imgH, transparentBackground);
}

QImage View::drawToImage(Time t, double x, double y, double w, double h, int imgW, int imgH, bool transparentBackground)
{
    // Test availability of OpenGL functionality
    if(!GLEW_VERSION_2_0) {
        qDebug("Error: OpenGL 2.0 not supported");
        return QImage();
    }
    if(!glewIsSupported("GL_ARB_framebuffer_object")) {
        qDebug("Error: GL_ARB_framebuffer_object not supported");
        return QImage();
    }

    // Convenient alias
    GLuint IMG_SIZE_X = imgW;
    GLuint IMG_SIZE_Y = imgH;

    // Make this widget's rendering context the current OpenGL context
    makeCurrent();


    // ------------ Create multisample FBO --------------------

    GLuint ms_fboId;
    GLuint ms_ColorBufferId;
    GLuint ms_DepthBufferId;
    GLint  ms_samples;

    // Maximum supported samples
    glGetIntegerv(GL_MAX_SAMPLES, &ms_samples);
    // Create FBO
    glGenFramebuffers(1, &ms_fboId);
    glBindFramebuffer(GL_FRAMEBUFFER, ms_fboId);
    // Create multisample color buffer
    glGenRenderbuffers(1, &ms_ColorBufferId);
    glBindRenderbuffer(GL_RENDERBUFFER, ms_ColorBufferId);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, ms_samples, GL_RGBA8, IMG_SIZE_X, IMG_SIZE_Y);
    // Create multisample depth buffer
    glGenRenderbuffers(1, &ms_DepthBufferId);
    glBindRenderbuffer(GL_RENDERBUFFER, ms_DepthBufferId);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, ms_samples, GL_DEPTH_COMPONENT24, IMG_SIZE_X, IMG_SIZE_Y);
    // Attach render buffers to FBO
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, ms_ColorBufferId);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, ms_DepthBufferId);
    // Check FBO status
    GLenum ms_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(ms_status != GL_FRAMEBUFFER_COMPLETE) {
        qDebug() << "Error: FBO ms_status != GL_FRAMEBUFFER_COMPLETE";
        return QImage();
    }


    // ------------ Create standard FBO --------------------

    GLuint fboId;
    GLuint textureId;
    GLuint rboId;

    // Create FBO
    glGenFramebuffers(1, &fboId);
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);
    // Create color texture
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, IMG_SIZE_X, IMG_SIZE_Y, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    // Create depth buffer
    glGenRenderbuffers(1, &rboId);
    glBindRenderbuffer(GL_RENDERBUFFER, rboId);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, IMG_SIZE_X, IMG_SIZE_Y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    // Attach render buffers / textures to FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboId);
    // Check FBO status
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE) {
        qDebug() << "Error: FBO status != GL_FRAMEBUFFER_COMPLETE";
        return QImage();
    }


    // ------------ Render scene to multisample FBO --------------------

    // Bind FBO
    glBindFramebuffer(GL_FRAMEBUFFER, ms_fboId);

    // Set viewport size
    double oldViewportWidth = viewportWidth_;
    double oldViewportHeight = viewportHeight_;
    viewportWidth_ = IMG_SIZE_X;
    viewportHeight_ = IMG_SIZE_Y;
    glViewport(0, 0, viewportWidth_, viewportHeight_);

    // Clear buffers
    glClearColor(0.0, 0.0, 0.0, 0.0); // Important: full black + transparency
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set projection matrix
    // Note: (0,h) and not (h,0) since y-axis is down in VPaint, up in QImage
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, 0, h, 0, 1);

    // Set view matrix
    glMatrixMode (GL_MODELVIEW);
    GLWidget_Camera2D camera2d;
    camera2d.setX(-x);
    camera2d.setY(-y);
    camera2d.setZoom(1);
    glLoadMatrixd(camera2d.viewMatrixData());

    // Draw scene
    ViewSettings::DisplayMode oldDM = viewSettings_.displayMode();
    viewSettings_.setDisplayMode(ViewSettings::ILLUSTRATION);
    viewSettings_.setDrawBackground(!transparentBackground);
    viewSettings_.setMainDrawing(false);
    viewSettings_.setDrawCursor(false);
    scene_->draw(t, viewSettings_);
    viewSettings_.setDrawCursor(true);
    viewSettings_.setDisplayMode(oldDM);

    // Restore viewport size
    viewportWidth_ = oldViewportWidth;
    viewportHeight_ = oldViewportHeight;
    glViewport(0, 0, viewportWidth_, viewportHeight_);

    // Unbind FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // ------ Blit multisample FBO to standard FBO ---------

    // Bind multisample FBO for reading
    glBindFramebuffer(GL_READ_FRAMEBUFFER, ms_fboId);
    // Bind standard FBO for drawing
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboId);
    // Blit
    glBlitFramebuffer(0, 0, IMG_SIZE_X, IMG_SIZE_Y, 0, 0, IMG_SIZE_X, IMG_SIZE_Y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    // Unbind FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // ------ Read standard FBO to RAM data ---------

    // Bind standard FBO for reading
    glBindTexture(GL_TEXTURE_2D, textureId);
    // Read
    uchar * img = new uchar[4 * IMG_SIZE_X * IMG_SIZE_Y];
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE,  img);
    // Unbind FBO
    glBindTexture(GL_TEXTURE_2D, 0);


    // ------ Release allocated GPU memory  ---------

    glDeleteFramebuffers(1, &ms_fboId);
    glDeleteRenderbuffers(1, &ms_ColorBufferId);
    glDeleteRenderbuffers(1, &ms_DepthBufferId);
    glDeleteFramebuffers(1, &fboId);
    glDeleteRenderbuffers(1, &rboId);
    glDeleteTextures(1, &textureId);


    // ------ Fix black border bleeding ---------

    // Notations:
    //     RGBA_old:  RGBA value already stored in frame buffer
    //     RGBA:      RGBA value of fragment incoming to frame buffer
    //     RGBA_new:  RGBA value to be stored in frame buffer after blending RGBA_old and RGBA
    //
    // The issue is caused by glBlendFunc which is kinda dumb. Indeed, when
    // A_old (the alpha that's already in the frame buffer) is 0, i.e. no fragment
    // have ever touched this pixel yet, then we don't want to use the equation:
    //                     RGBA_new = RGBA*A + RGBA_old*(1-A)
    // Instead, we really want:
    //                     RGBA_new = RGBA
    // since the RGB_old value is irrelevant (it's black because it has to be smth,
    // but really, it is "undefined", and it shouldn't darken the source RGB). More
    // Generally, the actual equations should be:
    //                     RGB_new  = RGB*A + RGB_old*(1-A)*A_old
    //                     A_new    = A     + (1-A)*A_old
    // For some reasons that I cannot explain, these equations are not definable by
    // glBlendFunc, while actually, it really should be the default. As a workaround,
    // I fix this as a post-processing step, and it seems to work well.

    // How does the fix work?
    //
    // If a pixel has, say, a RGBA final value of   (0.8, 0, 0, 0.8), it is likely that
    // it was originally a transparent red fragment (1  , 0, 0, 0.8) --or opaque red border
    // fragment with anti-aliasing-- which has been blended with the "black" background (0,0,0,0).
    // Therefore, I need to map back the red "0.8" to "1", i.e. multiply all RGB values by the
    // inverse of the alpha value 0.8. Then we clamp it to [0,1] to make sure we don't overflow.

    if(transparentBackground)
    {
        for(uint k=0; k<IMG_SIZE_X*IMG_SIZE_Y; ++k)
        {
            uchar * pixel = &(img[4*k]);
            double a = pixel[3];
            if( 0 < a && a < 255 )
            {
                double s = 255.0 / a;
                pixel[0] = (uchar) (std::min(255.0,std::floor(0.5+s*pixel[0])));
                pixel[1] = (uchar) (std::min(255.0,std::floor(0.5+s*pixel[1])));
                pixel[2] = (uchar) (std::min(255.0,std::floor(0.5+s*pixel[2])));
            }
        }
    }


    // ------ Convert to Qimage ---------

    // Create cleanup info to delete[] img when appropriate
    QImageCleanupFunction cleanupFunction = &imageCleanupHandler;
    void * cleanupInfo = reinterpret_cast<void*>(img);

    // Create QImage
    QImage res(img, IMG_SIZE_X, IMG_SIZE_Y, QImage::Format_RGBA8888, cleanupFunction, cleanupInfo);

    // Return QImage
    return res;
}

void View::updatePicking()
{
    // Remove previously highlighted object
    hoveredObject_ = Picking::Object();

    if(!pickingIsEnabled_)
        return;

    // Make this widget's rendering context the current OpenGL context
    makeCurrent();

    // get the viewport size, allocate memory if necessary
    GLint m_viewport[4];
    glGetIntegerv( GL_VIEWPORT, m_viewport );
    if( !(m_viewport[2]>0) || !(m_viewport[3]>0))
    {
        deletePicking();
        return;
    }
    else if(
        pickingImg_
        && (WINDOW_SIZE_X_ == (uint)m_viewport[2])
        && (WINDOW_SIZE_Y_ == (uint)m_viewport[3]))
    {
        // necessary objects already created: do nothing
    }
    else
    {
        deletePicking();
        WINDOW_SIZE_X_ = m_viewport[2];
        WINDOW_SIZE_Y_ = m_viewport[3];
        newPicking();
    }

    // set rendering destination to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, fboId_);

    // clear buffers
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Should we setup other things? (e.g., disabling antialiasing)
    // Seems to work as is. If issues, check GLWidget::initilizeGL()

    // Setup camera position and orientation
    setCameraPositionAndOrientation();

    // draw the picking
    drawPick();

    // unbind FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // extract the texture info from GPU to RAM: EXPENSIVE + MAY CAUSE OPENGL STALL
    glBindTexture(GL_TEXTURE_2D, textureId_);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pickingImg_);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Update highlighted object
    if(underMouse())
    {
        updateHoveredObject(mouse_Event_X_, mouse_Event_Y_);
    }
}
