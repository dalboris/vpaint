# Copyright (C) 2012-2016 The VPaint Developers.
# See the COPYRIGHT file at the top-level directory of this distribution
# and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
#
# This file is part of VPaint, a vector graphics editor. It is subject to the
# license terms and conditions in the LICENSE.MIT file found in the top-level
# directory of this distribution and at http://opensource.org/licenses/MIT

TEMPLATE = lib
QT += widgets opengl network

THIRD_DEPENDS = \
    GLEW \
    Eigen

LIB_DEPENDS = \
    OpenVac

# GLU (XXX to be removed)
unix:!macx: LIBS += -lGLU

# App version
VERSION = 1.6
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

# App resources
RESOURCES += VPaint.qrc

# Names for control/command modifier key
macx: DEFINES += ACTION_MODIFIER_NAME_SHORT=\\\"Cmd\\\" ACTION_MODIFIER_NAME=\\\"Command\\\"
else: DEFINES += ACTION_MODIFIER_NAME_SHORT=\\\"Ctrl\\\" ACTION_MODIFIER_NAME=\\\"Control\\\"

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
    MultiView.h \
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
    OpenGL/OpenGLWidget.h \
    Views/View.h \
    Views/View2D.h \
    Views/View2DMouseEvent.h \
    Views/View2DMouseAction.h \
    Views/ViewMouseEvent.h \
    Core/Cache.h \
    Views/ViewMouseAction.h \
    Core/Memory.h \
    ViewOld.h \
    SceneOld.h \
    Scene/SceneData.h \
    Scene/Scene.h \
    OpenGL/OpenGLRenderer.h \
    OpenGL/OpenGLSharedResources.h \
    OpenGL/OpenGLFunctions.h \
    Views/View2DRenderer.h \
    Scene/SceneRenderer.h \
    Scene/SceneRendererSharedResources.h \
    Layer/Layer.h \
    Core/DataObject.h \
    Layer/LayerData.h \
    Vac/Vac.h \
    Vac/VacData.h \
    Tools/Sketch/SketchAction.h \
    Cameras/Camera2D.h \
    Cameras/Camera2DData.h \
    Tools/View2D/PanView2DAction.h \
    Tools/View2D/ZoomView2DAction.h \
    Tools/View2D/RotateView2DAction.h

SOURCES += \
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
    MultiView.cpp \
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
    OpenGL/OpenGLWidget.cpp \
    Views/View.cpp \
    Views/View2D.cpp \
    Views/View2DMouseEvent.cpp \
    Views/View2DMouseAction.cpp \
    Views/ViewMouseEvent.cpp \
    Views/ViewMouseAction.cpp \
    ViewOld.cpp \
    SceneOld.cpp \
    Scene/SceneData.cpp \
    Scene/Scene.cpp \
    OpenGL/OpenGLRenderer.cpp \
    OpenGL/OpenGLSharedResources.cpp \
    Views/View2DRenderer.cpp \
    Scene/SceneRenderer.cpp \
    Scene/SceneRendererSharedResources.cpp \
    Layer/Layer.cpp \
    Layer/LayerData.cpp \
    Vac/Vac.cpp \
    Vac/VacData.cpp \
    Tools/Sketch/SketchAction.cpp \
    Cameras/Camera2D.cpp \
    Cameras/Camera2DData.cpp \
    Tools/View2D/PanView2DAction.cpp \
    Tools/View2D/ZoomView2DAction.cpp \
    Tools/View2D/RotateView2DAction.cpp

DISTFILES += \
    OpenGL/Shaders/Helloworld.v.glsl \
    OpenGL/Shaders/Helloworld.f.glsl

include($$OUT_PWD/.config.pri)
