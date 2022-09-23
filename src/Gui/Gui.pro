# Copyright (C) 2012-2019 The VPaint Developers.
# See the COPYRIGHT file at the top-level directory of this distribution
# and at https:#github.com/dalboris/vpaint/blob/master/COPYRIGHT
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http:#www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Qt configuration
TEMPLATE = app
TARGET = VPaint
CONFIG += qt c++11
QT += opengl openglextensions network

# App version
#
# Note: if version.txt changes, you need to explicitly run the following
# for the change to be taken into account:
#
# Linux/macOS:
#   make qmake
#   make clean
#   make
#
# Windows:
#   nmake qmake_all
#   nmake clean
#   nmake
#
VERSION = "$$cat(../version.txt)"
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

# OpenGL
win32 {
    LIBS += opengl32.lib
}

# GLU
unix:!macx: LIBS += -lGLU
win32 {
    LIBS += glu32.lib
}

###############################################################################
#                      SHIPPED EXTERNAL LIBRARIES

# Add shipped external libraries to includepath and dependpath
INCLUDEPATH += $$PWD/../Third/
DEPENDPATH += $$PWD/../Third/
!win32: QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_ISYSTEM

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

# So that files in Gui can do e.g. #include <VAC/Layer.h>
INCLUDEPATH += $$PWD/../

HEADERS += ../VAC/MainWindow.h \
    ../VAC/SaveAndLoad.h \
    ../VAC/Picking.h \
    ../VAC/Random.h \
    ../VAC/GLUtils.h \
    ../VAC/GLWidget.h \
    ../VAC/GLWidget_Camera.h \
    ../VAC/GLWidget_Camera2D.h \
    ../VAC/GLWidget_Material.h \
    ../VAC/GLWidget_Light.h \
    ../VAC/GeometryUtils.h \
    ../VAC/SceneObject.h \
    ../VAC/SceneObject_Example.h \
    ../VAC/SceneObjectVisitor.h \
    ../VAC/KeyFrame.h \
    ../VAC/Scene.h \
    ../VAC/MultiView.h \
    ../VAC/View.h \
    ../VAC/View3D.h \
    ../VAC/Timeline.h \
    ../VAC/Global.h \
    ../VAC/ColorSelector.h \
    ../VAC/SpinBox.h \
    ../VAC/VectorAnimationComplex/Cell.h \
    ../VAC/VectorAnimationComplex/SplitMap.h \
    ../VAC/VectorAnimationComplex/Eigen.h \
    ../VAC/VectorAnimationComplex/Intersection.h \
    ../VAC/VectorAnimationComplex/KeyFace.h \
    ../VAC/VectorAnimationComplex/KeyEdge.h \
    ../VAC/VectorAnimationComplex/Halfedge.h \
    ../VAC/VectorAnimationComplex/ForwardDeclaration.h \
    ../VAC/VectorAnimationComplex/KeyCell.h \
    ../VAC/VectorAnimationComplex/FaceCell.h \
    ../VAC/VectorAnimationComplex/EdgeCell.h \
    ../VAC/VectorAnimationComplex/VertexCell.h \
    ../VAC/VectorAnimationComplex/KeyVertex.h \
    ../VAC/VectorAnimationComplex/EdgeGeometry.h \
    ../VAC/VectorAnimationComplex/CellList.h \
    ../VAC/VectorAnimationComplex/CellVisitor.h \
    ../VAC/VectorAnimationComplex/Operators.h \
    ../VAC/VectorAnimationComplex/Operator.h \
    ../VAC/VectorAnimationComplex/SculptCurve.h \
    ../VAC/VectorAnimationComplex/ProperCycle.h \
    ../VAC/VectorAnimationComplex/ProperPath.h \
    ../VAC/VectorAnimationComplex/CycleHelper.h \
    ../VAC/VectorAnimationComplex/ZOrderedCells.h \
    ../VAC/VectorAnimationComplex/EdgeSample.h \
    ../VAC/VectorAnimationComplex/Algorithms.h \
    ../VAC/VectorAnimationComplex/SmartKeyEdgeSet.h \
    ../VAC/OpenGL.h \
    ../VAC/VectorAnimationComplex/Triangles.h \
    ../VAC/SelectionInfoWidget.h \
    ../VAC/VectorAnimationComplex/Cycle.h \
    ../VAC/VectorAnimationComplex/Path.h \
    ../VAC/VectorAnimationComplex/AnimatedVertex.h \
    ../VAC/VectorAnimationComplex/AnimatedCycle.h \
    ../VAC/VectorAnimationComplex/CellLinkedList.h \
    ../VAC/VectorAnimationComplex/HalfedgeBase.h \
    ../VAC/VectorAnimationComplex/KeyHalfedge.h \
    ../VAC/ViewSettings.h \
    ../VAC/View3DSettings.h \
    ../VAC/ObjectPropertiesWidget.h \
    ../VAC/AnimatedCycleWidget.h \
    ../VAC/VectorAnimationComplex/CellObserver.h \
    ../VAC/Color.h \
    ../VAC/DevSettings.h \
    ../VAC/Settings.h \
    ../VAC/SettingsDialog.h \
    ../VAC/VectorAnimationComplex/InbetweenCell.h \
    ../VAC/VectorAnimationComplex/InbetweenEdge.h \
    ../VAC/VectorAnimationComplex/InbetweenFace.h \
    ../VAC/VectorAnimationComplex/InbetweenHalfedge.h \
    ../VAC/VectorAnimationComplex/InbetweenVertex.h \
    ../VAC/VectorAnimationComplex/VAC.h \
    ../VAC/XmlStreamWriter.h \
    ../VAC/XmlStreamReader.h \
    ../VAC/CssColor.h \
    ../VAC/TimeDef.h \
    ../VAC/EditCanvasSizeDialog.h \
    ../VAC/ExportPngDialog.h \
    ../VAC/AboutDialog.h \
    ../VAC/ViewWidget.h \
    ../VAC/Background/Background.h \
    ../VAC/Background/BackgroundData.h \
    ../VAC/Background/BackgroundRenderer.h \
    ../VAC/Background/BackgroundWidget.h \
    ../VAC/Background/BackgroundUrlValidator.h \
    ../VAC/IO/FileVersionConverter.h \
    ../VAC/IO/XmlStreamTraverser.h \
    ../VAC/IO/XmlStreamConverter.h \
    ../VAC/IO/XmlStreamConverters/XmlStreamConverter_1_0_to_1_6.h \
    ../VAC/IO/FileVersionConverterDialog.h \
    ../VAC/Version.h \
    ../VAC/VectorAnimationComplex/BoundingBox.h \
    ../VAC/VectorAnimationComplex/TransformTool.h \
    ../VAC/LayersWidget.h \
    ../VAC/Layer.h \
    ../VAC/SvgParser.h \
    ../VAC/SvgImportDialog.h \
    ../VAC/SvgImportParams.h \
    Application.h \
    UpdateCheckDialog.h \
    UpdateCheck.h

SOURCES += main.cpp \
    ../VAC/SaveAndLoad.cpp \
    ../VAC/Picking.cpp \
    ../VAC/Random.cpp \
    ../VAC/GLUtils.cpp  \
    ../VAC/GLWidget.cpp  \
    ../VAC/MainWindow.cpp \
    ../VAC/GeometryUtils.cpp \
    ../VAC/SceneObject.cpp \
    ../VAC/SceneObjectVisitor.cpp \
    ../VAC/KeyFrame.cpp \
    ../VAC/Scene.cpp \
    ../VAC/MultiView.cpp \
    ../VAC/View.cpp \
    ../VAC/View3D.cpp \
    ../VAC/Timeline.cpp \
    ../VAC/Global.cpp \
    ../VAC/ColorSelector.cpp \
    ../VAC/SpinBox.cpp \
    ../VAC/VectorAnimationComplex/Intersection.cpp \
    ../VAC/VectorAnimationComplex/Cell.cpp \
    ../VAC/VectorAnimationComplex/KeyCell.cpp \
    ../VAC/VectorAnimationComplex/KeyFace.cpp \
    ../VAC/VectorAnimationComplex/KeyEdge.cpp \
    ../VAC/VectorAnimationComplex/Halfedge.cpp \
    ../VAC/VectorAnimationComplex/FaceCell.cpp \
    ../VAC/VectorAnimationComplex/EdgeCell.cpp \
    ../VAC/VectorAnimationComplex/VertexCell.cpp \
    ../VAC/VectorAnimationComplex/KeyVertex.cpp \
    ../VAC/VectorAnimationComplex/EdgeGeometry.cpp \
    ../VAC/VectorAnimationComplex/CellVisitor.cpp \
    ../VAC/VectorAnimationComplex/Operators.cpp \
    ../VAC/VectorAnimationComplex/Operator.cpp \
    ../VAC/VectorAnimationComplex/ProperCycle.cpp \
    ../VAC/VectorAnimationComplex/ProperPath.cpp \
    ../VAC/VectorAnimationComplex/CycleHelper.cpp \
    ../VAC/VectorAnimationComplex/ZOrderedCells.cpp \
    ../VAC/VectorAnimationComplex/EdgeSample.cpp \
    ../VAC/VectorAnimationComplex/Cycle.cpp \
    ../VAC/VectorAnimationComplex/Algorithms.cpp \
    ../VAC/VectorAnimationComplex/SmartKeyEdgeSet.cpp \
    ../VAC/VectorAnimationComplex/Triangles.cpp \
    ../VAC/SelectionInfoWidget.cpp \
    ../VAC/VectorAnimationComplex/Path.cpp \
    ../VAC/VectorAnimationComplex/AnimatedVertex.cpp \
    ../VAC/VectorAnimationComplex/AnimatedCycle.cpp \
    ../VAC/VectorAnimationComplex/CellLinkedList.cpp \
    ../VAC/VectorAnimationComplex/HalfedgeBase.cpp \
    ../VAC/VectorAnimationComplex/KeyHalfedge.cpp \
    ../VAC/ViewSettings.cpp \
    ../VAC/View3DSettings.cpp \
    ../VAC/ObjectPropertiesWidget.cpp \
    ../VAC/AnimatedCycleWidget.cpp \
    ../VAC/VectorAnimationComplex/CellObserver.cpp \
    ../VAC/Color.cpp \
    ../VAC/DevSettings.cpp \
    ../VAC/Settings.cpp \
    ../VAC/SettingsDialog.cpp \
    ../VAC/VectorAnimationComplex/InbetweenCell.cpp \
    ../VAC/VectorAnimationComplex/InbetweenEdge.cpp \
    ../VAC/VectorAnimationComplex/InbetweenFace.cpp \
    ../VAC/VectorAnimationComplex/InbetweenHalfedge.cpp \
    ../VAC/VectorAnimationComplex/InbetweenVertex.cpp \
    ../VAC/VectorAnimationComplex/VAC.cpp \
    ../VAC/XmlStreamWriter.cpp \
    ../VAC/XmlStreamReader.cpp \
    ../VAC/CssColor.cpp \
    ../VAC/TimeDef.cpp \
    ../VAC/EditCanvasSizeDialog.cpp \
    ../VAC/ExportPngDialog.cpp \
    ../VAC/AboutDialog.cpp \
    ../VAC/ViewWidget.cpp \
    ../VAC/Background/Background.cpp \
    ../VAC/Background/BackgroundData.cpp \
    ../VAC/Background/BackgroundRenderer.cpp \
    ../VAC/Background/BackgroundWidget.cpp \
    ../VAC/Background/BackgroundUrlValidator.cpp \
    ../VAC/IO/FileVersionConverter.cpp \
    ../VAC/IO/XmlStreamTraverser.cpp \
    ../VAC/IO/XmlStreamConverter.cpp \
    ../VAC/IO/XmlStreamConverters/XmlStreamConverter_1_0_to_1_6.cpp \
    ../VAC/IO/FileVersionConverterDialog.cpp \
    ../VAC/Version.cpp \
    ../VAC/VectorAnimationComplex/BoundingBox.cpp \
    ../VAC/VectorAnimationComplex/TransformTool.cpp \
    ../VAC/LayersWidget.cpp \
    ../VAC/Layer.cpp \
    ../VAC/SvgParser.cpp \
    ../VAC/SvgImportDialog.cpp \
    ../VAC/SvgImportParams.cpp \
    Application.cpp \
    UpdateCheckDialog.cpp \
    UpdateCheck.cpp
