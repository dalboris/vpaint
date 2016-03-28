// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "Global.h"

#include "DevSettings.h"
#include "SceneOld.h"
#include "ViewOld.h"
#include "MainWindow.h"
#include "SettingsDialog.h"
#include "DevSettings.h"
#include "ColorSelector.h"
#include "Timeline.h"

#include <QDebug>
#include <QApplication>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QPushButton>
#include <QToolBar>
#include <QMenu>
#include <QAction>
#include <QSettings>
#include <QStatusBar>
#include <QDir>

// -------- Initialization --------

Global * global_ = 0;
Global * global() { return global_; }
void Global::initialize(MainWindow * w) { global_ = new Global(w); }

Global::Global(MainWindow * w) :
    toolMode_(SELECT),
    toolBar_(0),
    toolModeToolBar_(0),
    isScalingCorner_(false),
    isScalingEdge_(false),
    isRotating_(false),
    isDragAndDropping_(false),
    isDraggingPivot_(false),
    xSceneCursorPos_(0),
    ySceneCursorPos_(0),
    currentDisplayMode_(ILLUSTRATION),
    switchToDisplayMode_(OUTLINE),
    otherDisplayMode_(ILLUSTRATION_OUTLINE),
    mainWindow_(w),
    preferences_(),
    preferencesDialog_(0),
    settings_(0),
    documentDir_(QDir::home())
{
    // Color selectors
    currentColor_ = new ColorSelector();
    currentColor_->setToolTip(tr("Current color (C)"));
    currentColor_->setStatusTip(tr("Click to open the color selector"));

    snapThreshold_ = new SpinBox();
    snapThreshold_->setCaption(tr(" snap threshold "));

    sculptRadius_ = new SpinBox();
    sculptRadius_->setCaption(tr(" sculpt radius "));

    // Event filter
    QCoreApplication::instance()->installEventFilter(this);

    // Status bar help
    statusBarHelp_ = new QLabel();
    statusBarHelp_->setText("Find help here.");
    w->statusBar()->addWidget(statusBarHelp_);
    connect(this, SIGNAL(keyboardModifiersChanged()), this, SLOT(updateStatusBarHelp()));

}

bool Global::deleteIsolatedVertices()
{
    return true;
}

bool Global::deleteShortEdges()
{
    return true;
}

Qt::KeyboardModifiers Global::keyboardModifiers()
{
    return keyboardModifiers_;
}

void Global::updateModifiers()
{
    Qt::KeyboardModifiers keyboardModifiers = QGuiApplication::queryKeyboardModifiers();
    if(keyboardModifiers_ != keyboardModifiers)
    {
        keyboardModifiers_ = keyboardModifiers;
        emit keyboardModifiersChanged();
    }
}

bool Global::eventFilter(QObject * /*watched*/, QEvent * event)
{
    // Every single event delivered by Qt go through this method first before
    // going to its target object, so keep it as lightweight as possible

    // It is used as a convenient way to fix a few event behaviours that were
    // not quite right out of the box.

    // --------------------- Detect modifier key presses --------------

    // Detect modifier key presses (Shift, Ctrl, Alt, etc.) and update application
    // state accordingly (e.g., indicate which modifiers are pressed in the status bar, or
    // redraw the scene, since highlighting color depends on which modifiers are pressed)

    // If a modifier is pressed or released, update the modifier state, and emit a signal
    // if this state has changed
    // If a modifier is pressed or released, update the modifier state, and emit a signal
    // if this state has changed
    if(event->type() == QEvent::KeyPress ||
       event->type() == QEvent::KeyRelease)
    {
        QKeyEvent * keyEvent = static_cast<QKeyEvent *>(event);
        if(keyEvent)
        {
            // Workaround for Mac delete key
            // This is needed because of a bug in QT 5 that has not been resolved as of 5.5.0
#ifdef Q_OS_MAC
            if(keyEvent->key() == Qt::Key_Backspace)
            {
                scene()->smartDelete();
            }
#endif
            if(keyEvent->key() == Qt::Key_Shift ||
               keyEvent->key() == Qt::Key_Alt ||
               keyEvent->key() == Qt::Key_Meta ||
               keyEvent->key() == Qt::Key_AltGr ||
               keyEvent->key() == Qt::Key_Control)
            {
                updateModifiers();
            }
        }

        // Continue normal processing of the event
        return false;
    }
    else if(event->type() == QEvent::FocusIn )
    {
        updateModifiers();

        // Continue normal processing of the event
        return false;
    }

    // --------------------- Resolve shortcut overloads --------------

    // Resolve shortcut overloads
    else if(event->type() == QEvent::Shortcut)
    {
        QShortcutEvent * shortcutEvent = static_cast<QShortcutEvent *>(event);

        if(shortcutEvent->isAmbiguous())
        {
            QKeySequence key = shortcutEvent->key();
            resolveAmbiguousShortcuts(key);

            // Stop processing of the event
            return true;

        }
        else
        {
            // Continue normal processing of the event
            return false;
        }
    }

    // --------------------- Keep standard behaviour --------------

    // Continue normal processing of the event
    return false;
}

void Global::resolveAmbiguousShortcuts(const QKeySequence & key)
{
    qDebug() << "Ambiguous shortcut:" << key;
}

Eigen::Vector2d Global::sceneCursorPos() const
{
    return Eigen::Vector2d(xSceneCursorPos_, ySceneCursorPos_);
}

void Global::setSceneCursorPos(const Eigen::Vector2d & pos)
{
    xSceneCursorPos_ = pos[0];
    ySceneCursorPos_ = pos[1];
}

QToolBar * Global::toolModeToolBar() const
{
    return toolModeToolBar_;
}

QToolBar * Global::toolBar() const
{
    return toolBar_;
}

void Global::createToolBars()
{
    // ----- Tool modes -----

    // Create toolbar
    toolBar_ = mainWindow()->addToolBar(tr("Toolbar"));
    toolBar_->setOrientation(Qt::Vertical);
    toolBar_->setMovable(false);
    mainWindow()->addToolBar(Qt::LeftToolBarArea, toolBar_);

    // Set toolbar size
    int iconWidth = 32;
    toolBar_->setIconSize(QSize(iconWidth,iconWidth));
    currentColor_->setIconSize(QSize(iconWidth,iconWidth));
    currentColor_->updateIcon();

    // Create actions (exclusive checkable)
    QActionGroup * actionGroup = new QActionGroup(this);
    for(int i=0; i<NUMBER_OF_TOOL_MODES; i++)
    {
        toolModeActions[i] = new ToolModeAction(static_cast<ToolMode>(i), actionGroup);
        toolModeActions[i]->setCheckable(true);
        toolModeActions[i]->setShortcutContext(Qt::ApplicationShortcut);
        toolBar_->addAction(toolModeActions[i]);
        connect(toolModeActions[i], SIGNAL(triggered(Global::ToolMode)),
                              this, SLOT(setToolMode(Global::ToolMode)));
    }

    // Select
    toolModeActions[SELECT]->setText(tr("Select and move (F1)"));
    toolModeActions[SELECT]->setIcon(QIcon(":/images/select.png"));
    toolModeActions[SELECT]->setStatusTip(tr("Select objects, move objects, glue objects together, and split curves."));
    toolModeActions[SELECT]->setShortcut(QKeySequence(Qt::Key_F1));

    // Sketch
    toolModeActions[SKETCH]->setText(tr("Sketch (F2)"));
    toolModeActions[SKETCH]->setIcon(QIcon(":/images/sketch.png"));
    toolModeActions[SKETCH]->setStatusTip(tr("Sketch curves."));
    toolModeActions[SKETCH]->setShortcut(QKeySequence(Qt::Key_F2));

    // Paint
    toolModeActions[PAINT]->setText(tr("Paint (F3)"));
    toolModeActions[PAINT]->setIcon(QIcon(":/images/paint.png"));
    toolModeActions[PAINT]->setStatusTip(tr("Paint an empty space or an existing object."));
    toolModeActions[PAINT]->setShortcut(QKeySequence(Qt::Key_F3));

    // Sculpt
    toolModeActions[SCULPT]->setText(tr("Sculpt (F4)"));
    toolModeActions[SCULPT]->setIcon(QIcon(":/images/sculpt.png"));
    toolModeActions[SCULPT]->setStatusTip(tr("Sculpt curves."));
    toolModeActions[SCULPT]->setShortcut(QKeySequence(Qt::Key_F4));

    // ----- Color selectors -----

    // Colors
    colorSelectorAction_ = toolBar_->addWidget(currentColor_);
    colorSelectorAction_->setText(tr("Color"));
    colorSelectorAction_->setToolTip(tr("Color (C)"));
    colorSelectorAction_->setStatusTip(tr("Click to open the color selector"));
    colorSelectorAction_->setShortcut(QKeySequence(Qt::Key_C));
    colorSelectorAction_->setShortcutContext(Qt::ApplicationShortcut);
    connect(colorSelectorAction_, SIGNAL(triggered()), currentColor_, SLOT(click()));

    // ----- Tool Options -----

    toolModeToolBar_ = new QToolBar("Action Bar");
    toolModeToolBar_->setIconSize(QSize(200,iconWidth));
    toolModeToolBar_->setMovable(false);
    mainWindow()->addToolBar(toolModeToolBar_);

    // ---------------------   Color   ------------------------

    actionChangeColor_ = new QAction(this);
    actionChangeColor_->setText(tr("Change color"));
    actionChangeColor_->setIcon(QIcon(":/images/change-color.png"));
    actionChangeColor_->setStatusTip(tr("Change the color of the selected cells"));
    //actionChangeColor_->setShortcut(QKeySequence(Qt::Key_C));
    actionChangeColor_->setShortcutContext(Qt::ApplicationShortcut);
    mainWindow()->addAction(actionChangeColor_);
    connect(actionChangeColor_, SIGNAL(triggered()), mainWindow()->sceneOld(), SLOT(changeColor()));

    // ---------------------   Edges   ------------------------

    actionChangeEdgeWidth_ = new QAction(this);
    actionChangeEdgeWidth_->setText(tr("Change edge width (W)"));
    actionChangeEdgeWidth_->setIcon(QIcon(":/images/change-width.png"));
    actionChangeEdgeWidth_->setStatusTip(tr("Change the width of the selected edges"));
    actionChangeEdgeWidth_->setShortcut(QKeySequence(Qt::Key_W));
    actionChangeEdgeWidth_->setShortcutContext(Qt::ApplicationShortcut);
    mainWindow()->addAction(actionChangeEdgeWidth_);
    connect(actionChangeEdgeWidth_, SIGNAL(triggered()), mainWindow()->sceneOld(), SLOT(changeEdgeWidth()));


    // ---------------------   Faces   ------------------------

    actionCreateFace_ = new QAction(this);
    actionCreateFace_->setText(tr("Create Face (F)"));
    actionCreateFace_->setIcon(QIcon(":/images/create-face.png"));
    actionCreateFace_->setStatusTip(tr("Create a face whose boundary is the selected edges"));
    actionCreateFace_->setShortcut(QKeySequence(Qt::Key_F));
    actionCreateFace_->setShortcutContext(Qt::ApplicationShortcut);
    mainWindow()->addAction(actionCreateFace_);
    connect(actionCreateFace_, SIGNAL(triggered()), mainWindow()->sceneOld(), SLOT(createFace()));

    actionAddCycles_ = new QAction(this);
    actionAddCycles_->setText(tr("Add Holes (H)"));
    actionAddCycles_->setIcon(QIcon(":/images/add-cycles.png"));
    actionAddCycles_->setStatusTip(tr("Add holes to the selected face, whose boundaries are the selected edges"));
    actionAddCycles_->setShortcut(QKeySequence(Qt::Key_H));
    actionAddCycles_->setShortcutContext(Qt::ApplicationShortcut);
    mainWindow()->addAction(actionAddCycles_);
    connect(actionAddCycles_, SIGNAL(triggered()), mainWindow()->sceneOld(), SLOT(addCyclesToFace()));

    actionRemoveCycles_ = new QAction(this);
    actionRemoveCycles_->setText(tr("Remove Holes (Ctrl+H)"));
    actionRemoveCycles_->setIcon(QIcon(":/images/remove-cycles.png"));
    actionRemoveCycles_->setStatusTip(tr("Remove holes from the selected face, whose boundaries are the selected edges"));
    actionRemoveCycles_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_H));
    actionRemoveCycles_->setShortcutContext(Qt::ApplicationShortcut);
    mainWindow()->addAction(actionRemoveCycles_);
    connect(actionRemoveCycles_, SIGNAL(triggered()), mainWindow()->sceneOld(), SLOT(removeCyclesFromFace()));

    // ---------------------   Topological operations   ------------------------

    actionGlue_ = new QAction(this);
    actionGlue_->setText(tr("Glue"));
    actionGlue_->setToolTip(tr("Glue (G)"));
    actionGlue_->setIcon(QIcon(":/images/glue.png"));
    actionGlue_->setStatusTip(tr("Glue two endpoints or two curves together"));
    actionGlue_->setShortcut(QKeySequence(Qt::Key_G));
    actionGlue_->setShortcutContext(Qt::ApplicationShortcut);
    mainWindow()->addAction(actionGlue_);
    connect(actionGlue_, SIGNAL(triggered()), mainWindow()->sceneOld(), SLOT(glue()));

    actionUnglue_ = new QAction(this);
    actionUnglue_->setText(tr("Explode"));
    actionUnglue_->setToolTip(tr("Explode (E)"));
    actionUnglue_->setIcon(QIcon(":/images/unglue.png"));
    actionUnglue_->setStatusTip(tr("Duplicate the selected objects to disconnect adjacent curves and surfaces"));
    //actionUnglue_->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_G));
    actionUnglue_->setShortcut(QKeySequence(Qt::Key_E));
    actionUnglue_->setShortcutContext(Qt::ApplicationShortcut);
    mainWindow()->addAction(actionUnglue_);
    connect(actionUnglue_, SIGNAL(triggered()), mainWindow()->sceneOld(), SLOT(unglue()));

    actionUncut_ = new QAction(this);
    actionUncut_->setText(tr("Simplify"));
    actionUncut_->setToolTip(tr("Simplify (Backspace)"));
    actionUncut_->setIcon(QIcon(":/images/simplify.png"));
    actionUncut_->setStatusTip(tr("Simplify the selected objects, by merging curves and surfaces together"));
    actionUncut_->setShortcut(QKeySequence(Qt::Key_Backspace));
    actionUncut_->setShortcutContext(Qt::ApplicationShortcut);
    mainWindow()->addAction(actionUncut_);
    connect(actionUncut_, SIGNAL(triggered()), mainWindow()->sceneOld(), SLOT(uncut()));

    // Desired icon size
    double sideLength = 40;

    // ---------------------   Shared actions/options   ------------------------

    // None

    // ---------------------   Select options   ------------------------

    toolModeToolBar_->addAction(actionChangeColor_);
    toolModeToolBar_->addAction(actionChangeEdgeWidth_);
    separatorSelect1_ = toolModeToolBar_->addSeparator();
    toolModeToolBar_->addAction(actionCreateFace_);
    toolModeToolBar_->addAction(actionAddCycles_);
    toolModeToolBar_->addAction(actionRemoveCycles_);
    separatorSelect2_ = toolModeToolBar_->addSeparator();

    toolModeToolBar_->widgetForAction(actionChangeColor_)->setFixedSize(sideLength,sideLength);
    toolModeToolBar_->widgetForAction(actionChangeEdgeWidth_)->setFixedSize(sideLength,sideLength);
    toolModeToolBar_->widgetForAction(actionCreateFace_)->setFixedSize(sideLength,sideLength);
    toolModeToolBar_->widgetForAction(actionAddCycles_)->setFixedSize(sideLength,sideLength);
    toolModeToolBar_->widgetForAction(actionRemoveCycles_)->setFixedSize(sideLength,sideLength);

    toolModeToolBar_->addAction(actionGlue_);
    toolModeToolBar_->addAction(actionUnglue_);
    toolModeToolBar_->addAction(actionUncut_);
    toolModeToolBar_->widgetForAction(actionGlue_)->setFixedSize(sideLength+20,sideLength);
    toolModeToolBar_->widgetForAction(actionUnglue_)->setFixedSize(sideLength+20,sideLength);
    toolModeToolBar_->widgetForAction(actionUncut_)->setFixedSize(sideLength+20,sideLength);


    // ---------------------   Sketch options   ------------------------

    // Tablet pressure
    actionUseTabletPressure_ = new QAction(this);
    actionUseTabletPressure_->setCheckable(true);
    actionUseTabletPressure_->setChecked(true);
    toolModeToolBar_->addAction(actionUseTabletPressure_);
    toolModeToolBar_->widgetForAction(actionUseTabletPressure_)->setFixedSize(sideLength,sideLength);
    actionUseTabletPressure_->setText(tr("Toggle stylus pressure"));
    actionUseTabletPressure_->setIcon(QIcon(":/images/pressure.png"));
    actionUseTabletPressure_->setStatusTip(tr("Enable or disable stylus pressure (only for users with a pen tablet)"));
    //actionUseTabletPressure_->setShortcut(QKeySequence(Qt::Key_Backspace));
    actionUseTabletPressure_->setShortcutContext(Qt::ApplicationShortcut);
    mainWindow()->addAction(actionUseTabletPressure_);
    connect(actionUseTabletPressure_, SIGNAL(triggered()), this, SLOT(toggleStylusPressure()));

    // Edge width
    edgeWidth_ = new SpinBox();
    edgeWidth_->setCaption(tr(" pen width "));
    edgeWidth_->setValue(settings().edgeWidth());
    actionEdgeWidth_ = toolModeToolBar_->addWidget(edgeWidth_);
    connect(edgeWidth_, SIGNAL(valueChanged(double)), this, SLOT(setEdgeWidth_(double)));

    // Separator
    separatorSketch1_ = toolModeToolBar_->addSeparator();

    // Planar map mode
    actionPlanarMapMode_ = new QAction(this);
    actionPlanarMapMode_->setCheckable(true);
    actionPlanarMapMode_->setChecked(true);
    toolModeToolBar_->addAction(actionPlanarMapMode_);
    toolModeToolBar_->widgetForAction(actionPlanarMapMode_)->setFixedSize(110,sideLength);
    actionPlanarMapMode_->setText(tr("Toggle intersections"));
    actionPlanarMapMode_->setIcon(QIcon(":/images/planar-map-on.png"));
    actionPlanarMapMode_->setStatusTip(tr("When intersections are enabled, the sketched curve automatically splits existing curves and surfaces."));
    //actionPlanarMapMode_->setShortcut(QKeySequence(Qt::Key_Backspace));
    actionPlanarMapMode_->setShortcutContext(Qt::ApplicationShortcut);
    mainWindow()->addAction(actionPlanarMapMode_);
    connect(actionPlanarMapMode_, SIGNAL(triggered()), this, SLOT(togglePlanarMapMode()));

    // Separator
    separatorSketch2_ = toolModeToolBar_->addSeparator();

    // Snapping
    actionSnapMode_ = new QAction(this);
    actionSnapMode_->setCheckable(true);
    actionSnapMode_->setChecked(true);
    toolModeToolBar_->addAction(actionSnapMode_);
    toolModeToolBar_->widgetForAction(actionSnapMode_)->setFixedSize(110,sideLength);
    actionSnapMode_->setText(tr("Toggle snapping"));
    actionSnapMode_->setIcon(QIcon(":/images/snapping-on.png"));
    actionSnapMode_->setStatusTip(tr("When snapping is enabled, the sketched curve is automatically glued to existing curves."));
    //actionSnapMode_->setShortcut(QKeySequence(Qt::Key_Backspace));
    actionSnapMode_->setShortcutContext(Qt::ApplicationShortcut);
    mainWindow()->addAction(actionSnapMode_);
    connect(actionSnapMode_, SIGNAL(triggered()), this, SLOT(toggleSnapping()));

    // Edge width
    actionSnapThreshold_ = toolModeToolBar_->addWidget(snapThreshold_);

    // ---------------------   Sculpt options   ------------------------

    actionSculptRadius_ = toolModeToolBar_->addWidget(sculptRadius_);

    // ---------------------   Cut options   ------------------------

    // Set default Tool Mode
    setToolMode(SKETCH);
}

Global::DisplayMode Global::displayMode() const
{
    return currentDisplayMode_;
}

ViewOld * Global::activeView() const
{
    return mainWindow()->activeView();
}

ViewOld * Global::hoveredView() const
{
    return mainWindow()->hoveredView();
}

Time Global::activeTime() const
{
    return activeView()->activeTime();
}

Timeline * Global::timeline() const
{
    return mainWindow()->timeline();
}

void Global::setDisplayMode(Global::DisplayMode mode)
{
    if(currentDisplayMode_ == mode)
    {
        // do nothing
    }
    else
    {
        currentDisplayMode_  = mode;
    }
}

bool Global::showCanvas() const
{
    return mainWindow()->isShowCanvasChecked();
}

void Global::togglePlanarMapMode()
{
    if(actionPlanarMapMode_->isChecked())
    {
        actionPlanarMapMode_->setText(tr("Disable intersections"));
        actionPlanarMapMode_->setIcon(QIcon(":/images/planar-map-on.png"));
    }
    else
    {
        actionPlanarMapMode_->setText(tr("Enable intersections"));
        actionPlanarMapMode_->setIcon(QIcon(":/images/planar-map-off.png"));
    }
}

void Global::toggleSnapping()
{
    if(actionSnapMode_->isChecked())
    {
        actionSnapMode_->setText(tr("Disable snapping"));
        actionSnapMode_->setIcon(QIcon(":/images/snapping-on.png"));
        actionSnapThreshold_->setEnabled(true);
    }
    else
    {
        actionSnapMode_->setText(tr("Enable snapping"));
        actionSnapMode_->setIcon(QIcon(":/images/snapping-off.png"));
        actionSnapThreshold_->setEnabled(false);
    }
}

void Global::toggleStylusPressure()
{
    // Nothing to do
}

void Global::setScalingCorner(bool b)
{
    isScalingCorner_ = b;
    updateStatusBarHelp();
}

void Global::setScalingEdge(bool b)
{
    isScalingEdge_ = b;
    updateStatusBarHelp();
}

void Global::setRotating(bool b)
{
    isRotating_ = b;
    updateStatusBarHelp();
}

void Global::setDragAndDropping(bool b)
{
    isDragAndDropping_ = b;
    updateStatusBarHelp();
}

void Global::setDraggingPivot(bool b)
{
    isDraggingPivot_ = b;
    updateStatusBarHelp();
}

Global::ToolMode Global::toolMode() const
{
    if(mainWindow()->isEditCanvasSizeVisible())
    {
        return EDIT_CANVAS_SIZE;
    }
    else
    {
        return toolMode_;
    }
}

void Global::setToolMode(Global::ToolMode mode)
{
    // Check consistency with action state
    if(!toolModeActions[mode]->isChecked())
        toolModeActions[mode]->setChecked(true);

    // Set member variable
    toolMode_ = mode;

    // Hide everything
    actionChangeColor_->setVisible(false);
    actionChangeEdgeWidth_->setVisible(false);
    actionCreateFace_->setVisible(false);
    actionAddCycles_->setVisible(false);
    actionRemoveCycles_->setVisible(false);

    toolModeToolBar_->removeAction(actionGlue_);
    toolModeToolBar_->removeAction(actionUnglue_);
    toolModeToolBar_->removeAction(actionUncut_);
    actionPlanarMapMode_->setVisible(false);
    actionSnapMode_->setVisible(false);
    actionUseTabletPressure_->setVisible(false);
    actionEdgeWidth_->setVisible(false);
    actionSnapThreshold_->setVisible(false);
    actionSculptRadius_->setVisible(false);
    separatorSelect1_->setVisible(false);
    separatorSelect2_->setVisible(false);
    separatorSketch1_->setVisible(false);
    separatorSketch2_->setVisible(false);

    // Desired icon size
    double sideLength = 40;

    // Show relevant
    switch(toolMode_)
    {
    case SELECT:
        actionChangeColor_->setVisible(true);
        actionChangeEdgeWidth_->setVisible(true);
        actionCreateFace_->setVisible(true);
        actionAddCycles_->setVisible(true);
        actionRemoveCycles_->setVisible(true);
        toolModeToolBar_->addAction(actionGlue_);
        toolModeToolBar_->addAction(actionUnglue_);
        toolModeToolBar_->addAction(actionUncut_);
        toolModeToolBar_->widgetForAction(actionGlue_)->setFixedSize(sideLength+20,sideLength);
        toolModeToolBar_->widgetForAction(actionUnglue_)->setFixedSize(sideLength+20,sideLength);
        toolModeToolBar_->widgetForAction(actionUncut_)->setFixedSize(sideLength+20,sideLength);
        separatorSelect1_->setVisible(true);
        separatorSelect2_->setVisible(true);
        break;
    case SKETCH:
        actionPlanarMapMode_->setVisible(true);
        actionSnapMode_->setVisible(true);
        actionUseTabletPressure_->setVisible(true);
        actionSnapThreshold_->setVisible(true);
        actionEdgeWidth_->setVisible(true);
        separatorSketch1_->setVisible(true);
        separatorSketch2_->setVisible(true);
        break;
    case SCULPT:
        actionSculptRadius_->setVisible(true);
        break;
    case PAINT:
        break;
    default:
        break;
    }

    // Even when there's no icon, make it high enough
    toolModeToolBar_->setMinimumHeight(50);

    // Update help
    updateStatusBarHelp();

    // Update scene
    mainWindow()->update();
    mainWindow()->updatePicking();
}


void Global::updateStatusBarHelp()
{
    Qt::KeyboardModifiers keys = keyboardModifiers();
    bool isCtrlDown = keys & Qt::ControlModifier;
    bool isShiftDown = keys & Qt::ShiftModifier;
    bool isAltDown = keys & Qt::AltModifier;

    QString message;
    if(isCtrlDown || isShiftDown || isAltDown)
    {
        message += "[";
        if(isCtrlDown)
        {
            message += QString(ACTION_MODIFIER_NAME_SHORT).toUpper();
            if(isShiftDown || isAltDown)
                message += ",";
        }
        if(isShiftDown)
        {
            message += "SHIFT";
            if(isAltDown)
                message += ",";
        }

        if(isAltDown)
        {
            message += "ALT";
        }
        message += "] ";
    }

    if (isScalingCorner_)
    {
        if(!isShiftDown)
            message += "Hold SHIFT to preserve proportions. ";
        if(!isAltDown)
            message += "Hold ALT to scale relative to center/pivot. ";
    }
    else if (isScalingEdge_)
    {
        if(!isAltDown)
            message += "Hold ALT to scale relative to center/pivot. ";
    }
    else if (isRotating_)
    {
        if(!isShiftDown)
            message += "Hold SHIFT to rotate by 45° only. ";
        if(!isAltDown)
            message += "Hold ALT to rotate relative to opposite corner. ";
    }
    else if (isDragAndDropping_)
    {
        if(!isShiftDown)
            message += "Hold SHIFT to constrain translation along 45° axes. ";
    }
    else if (isDraggingPivot_)
    {
        if(!isShiftDown)
            message += "Hold SHIFT to snap to center and corners of bounding box. ";
    }
    else if(toolMode() == SELECT)
    {
        if(!isCtrlDown && !isShiftDown && !isAltDown) {
            message += "Click to select highlighted object. Click on background to deselect all. Hold " + QString(ACTION_MODIFIER_NAME_SHORT).toUpper() + ", SHIFT, or ALT for more actions.";
        }
        else if(isCtrlDown && !isShiftDown && !isAltDown) {
            message += "Click on curve to insert end point. Click on face to insert point-in-face.";
        }
        else if(!isCtrlDown && isShiftDown && !isAltDown) {
            message += "Click to add highlighted object to the selection. Hold also ALT for different action.";
        }
        else if(!isCtrlDown && !isShiftDown && isAltDown) {
            message += "Click to remove highlighted object from the selection. Hold also SHIFT for different action.";
        }
        else if(!isCtrlDown && isShiftDown && isAltDown) {
            message += "Click to select unselected objects, or deselect selected objects.";
        }
        else {
            message += "No action available for this combination of keyboard modifiers.";
        }
    }
    else if(toolMode() == SKETCH)
    {
        if(!isCtrlDown && !isShiftDown && !isAltDown) {
            message += "Hold left mouse button to draw a curve. " + QString(ACTION_MODIFIER_NAME_SHORT).toUpper() + ": Change pen width. ALT: Change snap threshold.";
        }
        else if(isCtrlDown && !isShiftDown && !isAltDown) {
            message += "Hold left mouse button to change pen width.";
        }
        else if(!isCtrlDown && !isShiftDown && isAltDown) {
            message += "Hold left mouse button to change snap threshold.";
        }
        else if(isCtrlDown && !isShiftDown && isAltDown) {
            message += "Hold left mouse button to change both pen width and snap threshold.";
        }
        else {
            message += "No action available for this combination of keyboard modifiers.";
        }
    }
    else if(toolMode() == PAINT)
    {
        if(!isCtrlDown && !isShiftDown && !isAltDown) {
            message += "Click on closed region delimited by curves to fill. Click on object to change color. Click on background to change background color.";
        }
        else {
            message += "No action available for this combination of keyboard modifiers.";
        }
    }
    else if(toolMode() == SCULPT)
    {
        if(!isCtrlDown && !isShiftDown && !isAltDown) {
            message += "Hold left mouse button (LMB) to drag endpoint, or drag curve within radius. " + QString(ACTION_MODIFIER_NAME_SHORT).toUpper() + ": radius. SHIFT: smooth. ALT: thickness.";
        }
        else if(isCtrlDown && !isShiftDown && !isAltDown) {
            message += "Hold LMB to change the radius of the sculpting tool. Note: radius not visible if cursor too far from curve.";
        }
        else if(!isCtrlDown && isShiftDown && !isAltDown) {
            message += "Hold LMB to smooth curve within radius.";
        }
        else if(!isCtrlDown && !isShiftDown && isAltDown) {
            message += "Hold LMB to change thickness of curve within radius. Trick: use a large radius to edit thickness of the whole curve.";
        }
        else {
            message += "No action available for this combination of keyboard modifiers.";
        }
    }

    statusBarHelp_->setText(message);
}

// Other getters
DevSettings * Global::devSettings() { return DevSettings::instance(); }
MainWindow * Global::mainWindow() const { return mainWindow_; }
Settings & Global::settings() { return preferences_; }
VectorAnimationComplex::VAC * Global::currentVAC() { return mainWindow()->sceneOld()->getVAC_(); }
SceneOld * Global::scene() const {return mainWindow()->sceneOld();}

QColor Global::edgeColor()
{
    return currentColor_->color();
}

QColor Global::faceColor()
{
    return currentColor_->color();
}

bool Global::useTabletPressure() const
{
    return actionUseTabletPressure_->isChecked();
}

double Global::edgeWidth() const
{
    return edgeWidth_->value();
}

void Global::setEdgeWidth_(double w)
{
    preferences_.setEdgeWidth(w);
}

void Global::setEdgeWidth(double w)
{
    if(edgeWidth() != w)
    {
        edgeWidth_->setValue(w);
    }

    preferences_.setEdgeWidth(w);
}

void Global::openPreferencesDialog()
{
    // Create preferences dialog
    if(!preferencesDialog_)
    {
        preferencesDialog_ = new SettingsDialog(mainWindow());
        connect(preferencesDialog_, SIGNAL(preferencesChanged()), this, SLOT(updateWidgetValuesFromPreferences()));
    }

    // Update and show references dialog
    preferencesDialog_->go();
}

void Global::updateWidgetValuesFromPreferences()
{
    edgeWidth_->setValue(preferences_.edgeWidth());
}

bool Global::planarMapMode() const
{
    return actionPlanarMapMode_->isChecked();
}

bool Global::snapMode() const
{
    return actionSnapMode_->isChecked();
}

double Global::snapThreshold() const
{
    return snapThreshold_->value();
}

void Global::setSnapThreshold(double newSnapThreshold)
{
    return snapThreshold_->setValue(newSnapThreshold);
}

double Global::sculptRadius() const
{
    return sculptRadius_->value();
}

void Global::setSculptRadius(double newRadius)
{
    return sculptRadius_->setValue(newRadius);
}

void Global::readSettings()
{
    QSettings qsettings;

    // Geometry of the window
    QSize size = qsettings.value("size", QSize(400, 400)).toSize();
    QPoint pos = qsettings.value("pos", QPoint(200, 200)).toPoint();
    mainWindow()->resize(size);
    mainWindow()->move(pos);

    // User settings
    settings().readFromDisk(qsettings);

    // Other settings
    snapThreshold_->setValue( qsettings.value("tools-sketch-snapthreshold", 15.0).toDouble() );
    sculptRadius_->setValue( qsettings.value("tools-sculpt-radius", 50.0).toDouble() );
}

void Global::writeSettings()
{
      QSettings qsettings;

      // Geometry of the window
      qsettings.setValue("size", mainWindow()->size());
      qsettings.setValue("pos", mainWindow()->pos());

      // User settings
      settings().writeToDisk(qsettings);

      // Other settings
      qsettings.setValue("tools-sketch-snapthreshold", snapThreshold_->value() );
      qsettings.setValue("tools-sculpt-radius", sculptRadius_->value() );
}

// ----- ToolModeAction -----

ToolModeAction::ToolModeAction(Global::ToolMode mode, QObject * parent) :
    QAction(parent),
    toolMode(mode)
{
    connect(this, SIGNAL(triggered()), this, SLOT(emitSpecializedTriggered()));
}

void ToolModeAction::emitSpecializedTriggered()
{
    emit triggered(toolMode);
}

void Global::setDocumentDir(const QDir & dir)
{
    documentDir_ = dir;
}

QDir Global::documentDir() const
{
    return documentDir_;
}
