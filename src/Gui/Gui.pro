# Copyright (C) 2012-2018 The VPaint Developers.
# See the COPYRIGHT file at the top-level directory of this distribution
# and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
#
# This file is part of VPaint, a vector graphics editor. It is subject to the
# license terms and conditions in the LICENSE.MIT file found in the top-level
# directory of this distribution and at http://opensource.org/licenses/MIT

# This file is part of VPaint, a vector graphics editor.
#
# Copyright (C) 2015 Connor Deptuck (@scribblemaniac)
#
# The content of this file is MIT licensed. See COPYING.MIT, or this link:
#   http://opensource.org/licenses/MIT

# Qt configuration
TEMPLATE = app
TARGET = VPaint
CONFIG += qt c++11
QT += opengl network

# App version
MYVAR = 1.6
VERSION = $$MYVAR
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

# App resources
RESOURCES += resources.qrc

# App icon
win32 {
    # Set icon
    RC_ICONS += images/VPaint.ico
}
else:macx {
    # Set icon
    ICON = images/vpaint.icns

    # Use a custom Info.plist
    QMAKE_INFO_PLIST = Info.plist

    # Add file icons into the application bundle resources
    FILE_ICONS.files = images/vec.icns
    FILE_ICONS.path = Contents/Resources
    QMAKE_BUNDLE_DATA += FILE_ICONS
}

# Names for control/command modifier key
macx: DEFINES += ACTION_MODIFIER_NAME_SHORT=\\\"Cmd\\\" ACTION_MODIFIER_NAME=\\\"Command\\\"
else: DEFINES += ACTION_MODIFIER_NAME_SHORT=\\\"Ctrl\\\" ACTION_MODIFIER_NAME=\\\"Control\\\"

# Debug symbols
unix:!macx:CONFIG(debug, debug|release): QMAKE_CXXFLAGS += -gdwarf-2

# Windows only: embed manifest file
win32: CONFIG += embed_manifest_exe


###############################################################################
#                     UNSHIPPED EXTERNAL LIBRARIES

# GLU
unix:!macx: LIBS += -lGLU


###############################################################################
#                      SHIPPED EXTERNAL LIBRARIES

# Add shipped external libraries to includepath and dependpath
INCLUDEPATH += $$PWD/../Third/
DEPENDPATH += $$PWD/../Third/
!win32: QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_ISYSTEM $$PWD/../Third/

# Define RELEASE_OR_DEBUG convenient variable
CONFIG(release, debug|release): RELEASE_OR_DEBUG = release
CONFIG(debug,   debug|release): RELEASE_OR_DEBUG = debug

# GLEW
win32 {
    LIBS += -L$$OUT_PWD/../Third/GLEW/$$RELEASE_OR_DEBUG/ -lGLEW
    win32-g++: PRE_TARGETDEPS += $$OUT_PWD/../Third/GLEW/$$RELEASE_OR_DEBUG/libGLEW.a
    else:      PRE_TARGETDEPS += $$OUT_PWD/../Third/GLEW/$$RELEASE_OR_DEBUG/GLEW.lib
}
else:unix {
    LIBS += -L$$OUT_PWD/../Third/GLEW/ -lGLEW
    PRE_TARGETDEPS += $$OUT_PWD/../Third/GLEW/libGLEW.a
}


###############################################################################
#                            APP SOURCE FILES

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
    CssColor.h \
    TimeDef.h \
    EditCanvasSizeDialog.h \
    ExportPngDialog.h \
    AboutDialog.h \
    ViewMacOsX.h \
    Application.h \
    Background/Background.h \
    Background/BackgroundData.h \
    Background/BackgroundRenderer.h \
    Background/BackgroundWidget.h \
    Background/BackgroundUrlValidator.h \
    IO/FileVersionConverter.h \
    IO/XmlStreamTraverser.h \
    IO/XmlStreamConverter.h \
    IO/XmlStreamConverters/XmlStreamConverter_1_0_to_1_6.h \
    IO/FileVersionConverterDialog.h \
    UpdateCheckDialog.h \
    Version.h \
    UpdateCheck.h \
    VectorAnimationComplex/BoundingBox.h \
    VectorAnimationComplex/TransformTool.h \
    LayersWidget.h \
    Layer.h

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
    CssColor.cpp \
    TimeDef.cpp \
    EditCanvasSizeDialog.cpp \
    ExportPngDialog.cpp \
    AboutDialog.cpp \
    ViewMacOsX.cpp \
    Application.cpp \
    Background/Background.cpp \
    Background/BackgroundData.cpp \
    Background/BackgroundRenderer.cpp \
    Background/BackgroundWidget.cpp \
    Background/BackgroundUrlValidator.cpp \
    IO/FileVersionConverter.cpp \
    IO/XmlStreamTraverser.cpp \
    IO/XmlStreamConverter.cpp \
    IO/XmlStreamConverters/XmlStreamConverter_1_0_to_1_6.cpp \
    IO/FileVersionConverterDialog.cpp \
    UpdateCheckDialog.cpp \
    Version.cpp \
    UpdateCheck.cpp \
    VectorAnimationComplex/BoundingBox.cpp \
    VectorAnimationComplex/TransformTool.cpp \
    LayersWidget.cpp \
    Layer.cpp
