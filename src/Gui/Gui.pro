# This file is part of VPaint, a vector graphics editor.
#
# Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
# Copyright (C) 2015 Connor Deptuck (@scribblemaniac)
#
# The content of this file is MIT licensed. See COPYING.MIT, or this link:
#   http://opensource.org/licenses/MIT

# Basic Qt configuration
TEMPLATE = app
TARGET = VPaint
CONFIG += qt c++11
QT += opengl network

# Set the version and add the APP_VERSION macro for convenience
VERSION = 1.5
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

# To create the icon on Windows
win32: RC_FILE = VPaint.rc

# To create the icon on MacOS X
macx: ICON = images/vpaint.icns

QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_ISYSTEM $$PWD/../Third

# Compiler flags for Linux
unix:!macx {
  # Link to GLU
  LIBS += -lGLU
  # Use dwarf debug dymbols
  CONFIG(debug, debug|release) {
    QMAKE_CXXFLAGS += -gdwarf-2
  }
}

# Compiler flags for Mac OS X
macx {
  # Use a custom Info.plist
  QMAKE_INFO_PLIST = Info.plist

  # Add file icons into the application bundle resources
  FILE_ICONS.files = images/vec.icns
  FILE_ICONS.path = Contents/Resources
  QMAKE_BUNDLE_DATA += FILE_ICONS
}

# Compiler flags for Windows
win32 {
  # Embed the manifest file into the dll binary
  CONFIG += embed_manifest_exe
}

# App resources to include in binary (images, etc.)
RESOURCES 	+= resources.qrc

# Input
HEADERS += MainWindow.h \
    SaveAndLoad.h \
    Picking.h \
    Random.h \
    GLUtils.h \
    GLWidget.h \
    GLWidget_Settings.h \
    GLWidget_Camera.h \
    GLWidget_Camera2D.h \
    GLWidget_Material.h \
    GLWidget_Light.h \
    GeometryUtils.h \
    SceneObject.h \
    SceneObject_Example.h \
    SceneObjectVisitor.h \
    KeyFrame.h \
    Scene.h \
    MultiView.h \
    View.h \
    View3D.h \
    Timeline.h \
    Global.h \
    ColorSelector.h \
    SpinBox.h \
    VectorAnimationComplex/Cell.h \
    VectorAnimationComplex/SplitMap.h \
    VectorAnimationComplex/Eigen.h \
    VectorAnimationComplex/Intersection.h \
    VectorAnimationComplex/KeyFace.h \
    VectorAnimationComplex/KeyEdge.h \
    VectorAnimationComplex/Halfedge.h \
    VectorAnimationComplex/ForwardDeclaration.h \
    VectorAnimationComplex/KeyCell.h \
    VectorAnimationComplex/FaceCell.h \
    VectorAnimationComplex/EdgeCell.h \
    VectorAnimationComplex/VertexCell.h \
    VectorAnimationComplex/KeyVertex.h \
    VectorAnimationComplex/EdgeGeometry.h \
    VectorAnimationComplex/CellList.h \
    VectorAnimationComplex/CellVisitor.h \
    VectorAnimationComplex/Operators.h \
    VectorAnimationComplex/Operator.h \
    VectorAnimationComplex/SculptCurve.h \
    VectorAnimationComplex/ProperCycle.h \
    VectorAnimationComplex/ProperPath.h \
    VectorAnimationComplex/CycleHelper.h \
    VectorAnimationComplex/ZOrderedCells.h \
    VectorAnimationComplex/EdgeSample.h \
    VectorAnimationComplex/Algorithms.h \
    VectorAnimationComplex/SmartKeyEdgeSet.h \
    OpenGL.h \
    VectorAnimationComplex/Triangles.h \
    SelectionInfoWidget.h \
    VectorAnimationComplex/Cycle.h \
    VectorAnimationComplex/Path.h \
    VectorAnimationComplex/AnimatedVertex.h \
    VectorAnimationComplex/AnimatedCycle.h \
    VectorAnimationComplex/CellLinkedList.h \
    VectorAnimationComplex/HalfedgeBase.h \
    VectorAnimationComplex/KeyHalfedge.h \
    ViewSettings.h \
    View3DSettings.h \
    ObjectPropertiesWidget.h \
    AnimatedCycleWidget.h \
    VectorAnimationComplex/CellObserver.h \
    Color.h \
    DevSettings.h \
    Settings.h \
    SettingsDialog.h \
    VectorAnimationComplex/InbetweenCell.h \
    VectorAnimationComplex/InbetweenEdge.h \
    VectorAnimationComplex/InbetweenFace.h \
    VectorAnimationComplex/InbetweenHalfedge.h \
    VectorAnimationComplex/InbetweenVertex.h \
    VectorAnimationComplex/VAC.h \
    XmlStreamWriter.h \
    XmlStreamReader.h \
    CssStyle.h \
    CssColor.h \
    TimeDef.h \
    EditCanvasSizeDialog.h \
    ExportPngDialog.h \
    AboutDialog.h \
    ViewMacOsX.h \
    Application.h \
    Background.h \
    BackgroundWidget.h


SOURCES += main.cpp \
    SaveAndLoad.cpp \
    Picking.cpp \
    Random.cpp \
    GLUtils.cpp  \
    GLWidget.cpp  \
    GLWidget_Settings.cpp \
    MainWindow.cpp \
    GeometryUtils.cpp \
    SceneObject.cpp \
    SceneObjectVisitor.cpp \
    KeyFrame.cpp \
    Scene.cpp \
    MultiView.cpp \
    View.cpp \
    View3D.cpp \
    Timeline.cpp \
    Global.cpp \
    ColorSelector.cpp \
    SpinBox.cpp \
    VectorAnimationComplex/Intersection.cpp \
    VectorAnimationComplex/Cell.cpp \
    VectorAnimationComplex/KeyCell.cpp \
    VectorAnimationComplex/KeyFace.cpp \
    VectorAnimationComplex/KeyEdge.cpp \
    VectorAnimationComplex/Halfedge.cpp \
    VectorAnimationComplex/FaceCell.cpp \
    VectorAnimationComplex/EdgeCell.cpp \
    VectorAnimationComplex/VertexCell.cpp \
    VectorAnimationComplex/KeyVertex.cpp \
    VectorAnimationComplex/EdgeGeometry.cpp \
    VectorAnimationComplex/CellVisitor.cpp \
    VectorAnimationComplex/Operators.cpp \
    VectorAnimationComplex/Operator.cpp \
    VectorAnimationComplex/ProperCycle.cpp \
    VectorAnimationComplex/ProperPath.cpp \
    VectorAnimationComplex/CycleHelper.cpp \
    VectorAnimationComplex/ZOrderedCells.cpp \
    VectorAnimationComplex/EdgeSample.cpp \
    VectorAnimationComplex/Cycle.cpp \
    VectorAnimationComplex/Algorithms.cpp \
    VectorAnimationComplex/SmartKeyEdgeSet.cpp \
    VectorAnimationComplex/Triangles.cpp \
    SelectionInfoWidget.cpp \
    VectorAnimationComplex/Path.cpp \
    VectorAnimationComplex/AnimatedVertex.cpp \
    VectorAnimationComplex/AnimatedCycle.cpp \
    VectorAnimationComplex/CellLinkedList.cpp \
    VectorAnimationComplex/HalfedgeBase.cpp \
    VectorAnimationComplex/KeyHalfedge.cpp \
    ViewSettings.cpp \
    View3DSettings.cpp \
    ObjectPropertiesWidget.cpp \
    AnimatedCycleWidget.cpp \
    VectorAnimationComplex/CellObserver.cpp \
    Color.cpp \
    DevSettings.cpp \
    Settings.cpp \
    SettingsDialog.cpp \
    VectorAnimationComplex/InbetweenCell.cpp \
    VectorAnimationComplex/InbetweenEdge.cpp \
    VectorAnimationComplex/InbetweenFace.cpp \
    VectorAnimationComplex/InbetweenHalfedge.cpp \
    VectorAnimationComplex/InbetweenVertex.cpp \
    VectorAnimationComplex/VAC.cpp \
    XmlStreamWriter.cpp \
    XmlStreamReader.cpp \
    CssStyle.cpp \
    CssColor.cpp \
    TimeDef.cpp \
    EditCanvasSizeDialog.cpp \
    ExportPngDialog.cpp \
    AboutDialog.cpp \
    ViewMacOsX.cpp \
    Application.cpp \
    Background.cpp \
    BackgroundWidget.cpp

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../Third/GLEW/release/ -lGLEW
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../Third/GLEW/debug/ -lGLEW
else:unix: LIBS += -L$$OUT_PWD/../Third/GLEW/ -lGLEW

DEPENDPATH += $$PWD/../Third/GLEW

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Third/GLEW/release/libGLEW.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Third/GLEW/debug/libGLEW.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Third/GLEW/release/GLEW.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../Third/GLEW/debug/GLEW.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../Third/GLEW/libGLEW.a
