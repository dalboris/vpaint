// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QString>
#include <QTextBrowser>
#include <QTimer>
#include <QDir>

class QScrollArea;
class Scene;
class GLWidget;
class MultiView;
class View;
class View3D;
class Timeline;
class DevSettings;
class SettingsDialog;
class XmlStreamWriter;
class XmlStreamReader;
class QTextStream;
class EditCanvasSizeDialog;
class ExportPngDialog;
class AboutDialog;
class BackgroundWidget;

namespace VectorAnimationComplex
{
class VAC;
class InbetweenFace;
}
class SelectionInfoWidget;
class ObjectPropertiesWidget;
class AnimatedCycleWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

    Scene * scene() const;
    View * activeView() const;
    View * hoveredView() const;
    Timeline * timeline() const;

    bool isShowCanvasChecked() const;
    bool isEditCanvasSizeVisible() const;

protected:
    void closeEvent(QCloseEvent * event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    bool eventFilter(QObject *object, QEvent *event);

public slots:
    // ---- update what is displayed on screen ----
    void update();
    void updatePicking();

    void updateObjectProperties();
    void editAnimatedCycle(VectorAnimationComplex::InbetweenFace * inbetweenFace, int indexCycle);

    void about();
    bool doOpen(const QString & filename);

private slots:
    // ---- File ----
    bool newDocument();
    bool open();
    bool save();
    void autosave();
    bool saveAs();
    bool exportSVG();
    bool exportPNG();
    bool acceptExportPNG();
    bool rejectExportPNG();
    bool close();

    // ---- Edit ----
    void addToUndoStack();
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();

    // ---- View ----
    void displayModeChanged();
    void setDisplayModeNormal();
    void setDisplayModeNormalOutline();
    void setDisplayModeOutline();
    void toggleShowCanvas(bool);
    void editCanvasSize();

    void setOnionSkinningEnabled(bool enabled);

    void openClose3D();
    void updateView3DActionCheckState();
    void view3DActionSetChecked();
    void view3DActionSetUnchecked();

    void openClose3DSettings();
    void updateView3DSettingsActionCheckState();
    void view3DSettingsActionSetChecked();
    void view3DSettingsActionSetUnchecked();

    void updateViewMenu();

    // ---- Selection ----
    // -> deferred to Scene

    // ---- Depth ----
    // -> deferred to Scene

    // ---- Animation ----
    void motionPaste();
    void createInbetweenFace();

    // ---- Help ----
    void onlineDocumentation();
    void gettingStarted();
    void manual();
    
private:
    // ---------- initializations --------------
    void createActions();
    void createToolbars();
    void createStatusBar();
    void createDocks();
    void createMenus();

    // --------- Other properties and widgets --------
    // Scene and View
    Scene * scene_;
    MultiView * multiView_;
    // Help
    AboutDialog * aboutDialog_;
    bool showAboutDialogAtStartup_;
    QTextBrowser * gettingStarted_;
    QTextBrowser * userManual_;
    // Undo/Redo
    void goToUndoIndex_(int undoIndex);
    QList< QPair<QDir,Scene*> > undoStack_;
    int undoIndex_;
    // I/O
    QString fileHeader_;
    QString saveFilename_;
    QString autosaveFilename_;
    QTimer autosaveTimer_;
    int autosaveIndex_;
    bool autosaveOn_;
    QDir autosaveDir_;
    void setSaveFilename_(const QString & filename);
    bool doSave(const QString & filename, bool relativeRemap = false);
    bool doExportSVG(const QString & filename);
    bool doExportPNG(const QString & filename);
    void read_DEPRECATED(QTextStream & in);
    void write_DEPRECATED(QTextStream & out);
    void read(XmlStreamReader & xml);
    void write(XmlStreamWriter & xml);
    void autosaveBegin();
    void autosaveEnd();
    // Copy-pasting
    VectorAnimationComplex::VAC * clipboard_;
    // 3D view
    View3D * view3D_;
    // timeline
    Timeline * timeline_;
    // Selection info
    SelectionInfoWidget * selectionInfo_;
    // Edit Canvas Size
    ExportPngDialog * exportPngDialog_;
    EditCanvasSizeDialog * editCanvasSizeDialog_;
    bool exportPngCanvasWasVisible_;
    QString exportPngFilename_;
    bool exportingPng_;

    // --------- Menus and actions --------
    // FILE
    QMenu * menuFile;
      QAction * actionNew;
      QAction * actionOpen;
      QAction * actionSave;
      QAction * actionSaveAs;
      QAction * actionPreferences;
      QAction * actionExportSVG;
      QAction * actionExportPNG;
      QAction * actionQuit;
    // EDIT
    QMenu * menuEdit;
      QAction * actionUndo;
      QAction * actionRedo;
      QAction * actionCut;
      QAction * actionCopy;
      QAction * actionPaste;
      QAction * actionSmartDelete;
      QAction * actionHardDelete;
      QAction * actionTest;
    // VIEW
    QMenu * menuView;
      QAction * actionZoomIn;
      QAction * actionZoomOut;
      QAction * actionShowCanvas;
      QAction * actionEditCanvasSize;
      QAction * actionFitAllInWindow;
      QAction * actionFitSelectionInWindow;
      QAction * actionDisplayModeNormal;
      QAction * actionDisplayModeNormalOutline;
      QAction * actionDisplayModeOutline;
      QAction * actionOnionSkinning;
      QAction * actionToggleOutline;
      QAction * actionToggleOutlineOnly;
      QAction * actionOpenView3DSettings;
      QAction * actionOpenClose3D;
      QAction * actionSplitVertical;
      QAction * actionSplitHorizontal;
      QAction * actionSplitClose;
      QAction * actionSplitOne;
      QMenu * advancedViewMenu;
    // SELECTION
    QMenu * menuSelection;
      QAction * actionSelectAll;
      QAction * actionDeselectAll;
      QAction * actionInvertSelection;
      QAction * actionSelectConnected;
      QAction * actionSelectClosure;
      QAction * actionSelectVertices;
      QAction * actionSelectEdges;
      QAction * actionSelectFaces;
      QAction * actionDeselectVertices;
      QAction * actionDeselectEdges;
      QAction * actionDeselectFaces;
    // DEPTH
    QMenu * menuDepth;
      QAction * actionRaise;
      QAction * actionLower;
      QAction * actionRaiseToTop;
      QAction * actionLowerToBottom;
      QAction * actionAltRaise;
      QAction * actionAltLower;
      QAction * actionAltRaiseToTop;
      QAction * actionAltLowerToBottom;
    // ANIMATION
    QMenu * menuAnimation;
      QAction * actionInbetweenSelection;
      QAction * actionKeyframeSelection;
      QAction * actionMotionPaste;
      QAction * actionCreateInbetweenFace;
    // PLAYBACK
      QMenu * menuPlayback;
    // HELP
    QMenu * menuHelp;
      QAction * actionOnlineDocumentation;
      QAction * actionGettingStarted;
      QAction * actionManual;
      QAction * actionAbout;

    // Docks
    QDockWidget * dockInspector;
    ObjectPropertiesWidget * inspector;
    QDockWidget * dockTimeLine;
    QDockWidget * dockAdvancedSettings;
    QDockWidget * dockAnimatedCycleEditor;
    AnimatedCycleWidget * animatedCycleEditor;
    BackgroundWidget * backgroundWidget;
    QDockWidget * dockBackgroundWidget;
};

#endif
