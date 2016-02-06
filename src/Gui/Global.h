// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef GLOBAL_H
#define GLOBAL_H

// Define objects that we want accessible globally
//
// Example: global()->mainWindow()->update();
//          double w = global()->preferences().edgeWidth();

#include "Settings.h"

#include <QObject>
#include <QVector>
#include <QAction>
#include <QDoubleSpinBox>
#include "SpinBox.h"
#include "TimeDef.h"
#include <Eigen/Core>
#include <QLabel>
#include <QDir>

class QHBoxLayout;
class DevSettings;
class MainWindow;
class QToolBar;
class QAction;
class ToolModeAction;
class ColorSelector;
class QColor;
class SettingsDialog;
class View;
class Timeline;
class Scene;

namespace VectorAnimationComplex
{
class VAC;
}

class Global: public QObject
{
    Q_OBJECT

public:
    // Initialization
    static void initialize(MainWindow * w);
    Global(MainWindow * w);

    // Tool Mode
    void createToolBars();
    enum ToolMode {
        // Used for array indexes, don't change the numbers!
        SELECT = 0,
        SKETCH,
        PAINT,
        SCULPT,
        //CUT,
        NUMBER_OF_TOOL_MODES, // Keep this one last
        EDIT_CANVAS_SIZE // This one is below "Number of tools" as it's not a mode interface-wise
    };
    ToolMode toolMode() const;

    // Menus
    void addSelectionActions(QMenu * selectionMenu);

    // Keyboard state
    Qt::KeyboardModifiers keyboardModifiers();

    // Tablet pressure
    bool useTabletPressure() const;

    // Edge width
    double edgeWidth() const;
    void setEdgeWidth(double w);

    // Planar map mode
    bool planarMapMode() const;

    // snapping
    bool snapMode() const;
    double snapThreshold() const;
    void setSnapThreshold(double newSnapThreshold);

    // Sculpting
    double sculptRadius() const;
    void setSculptRadius(double newRadius);

    // Automatic topological cleaning
    bool deleteIsolatedVertices();
    bool deleteShortEdges();

    // Cursor position
    Eigen::Vector2d sceneCursorPos() const;
    void setSceneCursorPos(const Eigen::Vector2d & pos);

    // Colors
    QColor edgeColor();
    QColor faceColor();

    // Display modes
    enum DisplayMode {
        ILLUSTRATION,
        OUTLINE,
        ILLUSTRATION_OUTLINE
    };
    DisplayMode displayMode() const;
    void setDisplayMode(DisplayMode mode);
    bool showCanvas() const;

    // Active View and time
    View * activeView() const;
    View * hoveredView() const;
    Time activeTime() const;
    Timeline * timeline() const;

    // Other getters
    MainWindow * mainWindow() const;
    Scene * scene() const;
    Settings & settings();
    DevSettings * devSettings();

    VectorAnimationComplex::VAC * currentVAC();

    // Settings ( = user settings + application state )
    void readSettings();
    void writeSettings();

    // GUI elements owned by global
    QToolBar * toolModeToolBar() const;
    QToolBar * toolBar() const;

    // Directory from which paths in document are relative to
    void setDocumentDir(const QDir & dir);
    QDir documentDir() const;

signals:
    void keyboardModifiersChanged();

public slots:
    void setToolMode(Global::ToolMode mode);
    void togglePlanarMapMode();
    void toggleSnapping();
    void toggleStylusPressure();

    void setScalingCorner(bool b);
    void setScalingEdge(bool b);
    void setRotating(bool b);
    void setDragAndDropping(bool b);
    void setDraggingPivot(bool b);

    // Open preference dialog
    void openPreferencesDialog();

    // Update widgets
    void updateWidgetValuesFromPreferences();

    // Help message
    void updateStatusBarHelp();

protected:
    // Global event filter
    bool eventFilter(QObject * watched, QEvent * event);

    // Update keyboard modifiers
    void updateModifiers();

    // Resolve ambiguous shortcuts
    void resolveAmbiguousShortcuts(const QKeySequence & key);

private slots:
    void setEdgeWidth_(double w);


private:
    // Tools
    ToolModeAction * toolModeActions [NUMBER_OF_TOOL_MODES];

    // Color selector
    QAction * colorSelectorAction_;

    // Tool Mode
    ToolMode toolMode_;
    QToolBar * toolBar_;

    // Tool options
    QToolBar * toolModeToolBar_;

    // Is a selection being transformed?
    bool isScalingCorner_;
    bool isScalingEdge_;
    bool isRotating_;
    bool isDragAndDropping_;
    bool isDraggingPivot_;

    // Select
    QAction * actionChangeColor_;
    QAction * actionChangeEdgeWidth_;
    QAction * actionCreateFace_;
    QAction * actionAddCycles_;
    QAction * actionRemoveCycles_;
    QAction * actionGlue_;
    QAction * actionUnglue_;
    QAction * actionUncut_;
    // Sketch
    QAction * actionPlanarMapMode_;
    QAction * actionSnapMode_;
    SpinBox * edgeWidth_;
    QAction * actionEdgeWidth_;
    SpinBox * snapThreshold_;
    QAction * actionSnapThreshold_;
    QAction * actionUseTabletPressure_;
    // Sculpt
    SpinBox * sculptRadius_;
    QAction * actionSculptRadius_;

    // Separators
    QAction * separatorSelect1_;
    QAction * separatorSelect2_;
    QAction * separatorSketch1_;
    QAction * separatorSketch2_;
    QAction * separatorSketch3_;

    // Scene cursor pos
    double xSceneCursorPos_, ySceneCursorPos_;

    // Colors
    ColorSelector * currentColor_;

    // Display modes
    DisplayMode currentDisplayMode_;
    DisplayMode switchToDisplayMode_;
    DisplayMode otherDisplayMode_;
    QAction * actionSwitchDisplayMode_;
    QAction * actionSwitchToOtherDisplayMode_;

    // Others
    MainWindow * mainWindow_;
    Settings preferences_;
    SettingsDialog * preferencesDialog_;
    DevSettings * settings_;
    Qt::KeyboardModifiers keyboardModifiers_;
    QDir documentDir_;

    // Status bar help
    QLabel * statusBarHelp_;
};

class ToolModeAction: public QAction
{
    Q_OBJECT

public:
    ToolModeAction(Global::ToolMode mode, QObject * parent = 0);

signals:
    void triggered(Global::ToolMode mode);

private slots:
    void emitSpecializedTriggered();

private:
    Global::ToolMode toolMode;
};

Global * global();

#endif // GLOBAL_H
