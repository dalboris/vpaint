project(VAC)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(VAC_HEADER_FILES
    Background/Background.h
    Background/BackgroundData.h
    Background/BackgroundRenderer.h
    Background/BackgroundUrlValidator.h
    Background/BackgroundWidget.h
    IO/FileVersionConverter.h
    IO/FileVersionConverterDialog.h
    IO/XmlStreamConverter.h
    IO/XmlStreamTraverser.h
    IO/XmlStreamConverters/XmlStreamConverter_1_0_to_1_6.h
    VectorAnimationComplex/Algorithms.h
    VectorAnimationComplex/AnimatedCycle.h
    VectorAnimationComplex/AnimatedVertex.h
    VectorAnimationComplex/BoundingBox.h
    VectorAnimationComplex/Cell.h
    VectorAnimationComplex/CellLinkedList.h
    VectorAnimationComplex/CellList.h
    VectorAnimationComplex/CellObserver.h
    VectorAnimationComplex/CellVisitor.h
    VectorAnimationComplex/Cycle.h
    VectorAnimationComplex/CycleHelper.h
    VectorAnimationComplex/EdgeCell.h
    VectorAnimationComplex/EdgeGeometry.h
    VectorAnimationComplex/EdgeSample.h
    VectorAnimationComplex/Eigen.h
    VectorAnimationComplex/FaceCell.h
    VectorAnimationComplex/ForwardDeclaration.h
    VectorAnimationComplex/Halfedge.h
    VectorAnimationComplex/HalfedgeBase.h
    VectorAnimationComplex/InbetweenCell.h
    VectorAnimationComplex/InbetweenEdge.h
    VectorAnimationComplex/InbetweenFace.h
    VectorAnimationComplex/InbetweenHalfedge.h
    VectorAnimationComplex/InbetweenVertex.h
    VectorAnimationComplex/Intersection.h
    VectorAnimationComplex/KeyCell.h
    VectorAnimationComplex/KeyEdge.h
    VectorAnimationComplex/KeyFace.h
    VectorAnimationComplex/KeyHalfedge.h
    VectorAnimationComplex/KeyVertex.h
    VectorAnimationComplex/Operator.h
    VectorAnimationComplex/Operators.h
    VectorAnimationComplex/Path.h
    VectorAnimationComplex/ProperCycle.h
    VectorAnimationComplex/ProperPath.h
    VectorAnimationComplex/SculptCurve.h
    VectorAnimationComplex/SmartKeyEdgeSet.h
    VectorAnimationComplex/SplitMap.h
    VectorAnimationComplex/TransformTool.h
    VectorAnimationComplex/Triangles.h
    VectorAnimationComplex/VAC.h
    VectorAnimationComplex/VertexCell.h
    VectorAnimationComplex/ZOrderedCells.h
    AboutDialog.h
    AnimatedCycleWidget.h
    Color.h
    ColorSelector.h
    CssColor.h
    DevSettings.h
    EditCanvasSizeDialog.h
    ExportAsDialog.h
    ExportSettings.h
    FilePath.h
    GLUtils.h
    GLWidget.h
    GLWidget_Camera.h
    GLWidget_Camera2D.h
    GLWidget_Light.h
    GLWidget_Material.h
    GeometryUtils.h
    Global.h
    KeyFrame.h
    Layer.h
    LayersWidget.h
    MainWindow.h
    MultiView.h
    ObjectPropertiesWidget.h
    OpenGL.h
    Picking.h
    Random.h
    SaveAndLoad.h
    Scene.h
    SceneObject.h
    SceneObjectVisitor.h
    SceneObject_Example.h
    SelectionInfoWidget.h
    Settings.h
    SettingsDialog.h
    SpinBox.h
    SvgImportDialog.h
    SvgImportParams.h
    SvgParser.h
    TimeDef.h
    Timeline.h
    Version.h
    View.h
    View3D.h
    View3DSettings.h
    ViewSettings.h
    ViewWidget.h
    XmlStreamReader.h
    XmlStreamWriter.h)
set(VAC_SOURCE_FILES
    Background/Background.cpp
    Background/BackgroundData.cpp
    Background/BackgroundRenderer.cpp
    Background/BackgroundUrlValidator.cpp
    Background/BackgroundWidget.cpp
    IO/FileVersionConverter.cpp
    IO/FileVersionConverterDialog.cpp
    IO/XmlStreamConverter.cpp
    IO/XmlStreamTraverser.cpp
    IO/XmlStreamConverters/XmlStreamConverter_1_0_to_1_6.cpp
    VectorAnimationComplex/Algorithms.cpp
    VectorAnimationComplex/AnimatedCycle.cpp
    VectorAnimationComplex/AnimatedVertex.cpp
    VectorAnimationComplex/BoundingBox.cpp
    VectorAnimationComplex/Cell.cpp
    VectorAnimationComplex/CellLinkedList.cpp
    VectorAnimationComplex/CellObserver.cpp
    VectorAnimationComplex/CellVisitor.cpp
    VectorAnimationComplex/Cycle.cpp
    VectorAnimationComplex/CycleHelper.cpp
    VectorAnimationComplex/EdgeCell.cpp
    VectorAnimationComplex/EdgeGeometry.cpp
    VectorAnimationComplex/EdgeSample.cpp
    VectorAnimationComplex/FaceCell.cpp
    VectorAnimationComplex/Halfedge.cpp
    VectorAnimationComplex/HalfedgeBase.cpp
    VectorAnimationComplex/InbetweenCell.cpp
    VectorAnimationComplex/InbetweenEdge.cpp
    VectorAnimationComplex/InbetweenFace.cpp
    VectorAnimationComplex/InbetweenHalfedge.cpp
    VectorAnimationComplex/InbetweenVertex.cpp
    VectorAnimationComplex/Intersection.cpp
    VectorAnimationComplex/KeyCell.cpp
    VectorAnimationComplex/KeyEdge.cpp
    VectorAnimationComplex/KeyFace.cpp
    VectorAnimationComplex/KeyHalfedge.cpp
    VectorAnimationComplex/KeyVertex.cpp
    VectorAnimationComplex/Operator.cpp
    VectorAnimationComplex/Operators.cpp
    VectorAnimationComplex/Path.cpp
    VectorAnimationComplex/ProperCycle.cpp
    VectorAnimationComplex/ProperPath.cpp
    VectorAnimationComplex/SmartKeyEdgeSet.cpp
    VectorAnimationComplex/TransformTool.cpp
    VectorAnimationComplex/Triangles.cpp
    VectorAnimationComplex/VAC.cpp
    VectorAnimationComplex/VertexCell.cpp
    VectorAnimationComplex/ZOrderedCells.cpp
    AboutDialog.cpp
    AnimatedCycleWidget.cpp
    Color.cpp
    ColorSelector.cpp
    CssColor.cpp
    DevSettings.cpp
    EditCanvasSizeDialog.cpp
    ExportAsDialog.cpp
    ExportSettings.cpp
    FilePath.cpp
    GLUtils.cpp
    GLWidget.cpp
    GeometryUtils.cpp
    Global.cpp
    KeyFrame.cpp
    Layer.cpp
    LayersWidget.cpp
    MainWindow.cpp
    MultiView.cpp
    ObjectPropertiesWidget.cpp
    Picking.cpp
    Random.cpp
    SaveAndLoad.cpp
    Scene.cpp
    SceneObject.cpp
    SceneObjectVisitor.cpp
    SelectionInfoWidget.cpp
    Settings.cpp
    SettingsDialog.cpp
    SpinBox.cpp
    SvgImportDialog.cpp
    SvgImportParams.cpp
    SvgParser.cpp
    TimeDef.cpp
    Timeline.cpp
    Version.cpp
    View.cpp
    View3D.cpp
    View3DSettings.cpp
    ViewSettings.cpp
    ViewWidget.cpp
    XmlStreamReader.cpp
    XmlStreamWriter.cpp)
source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${VAC_HEADER_FILES} ${VAC_SOURCE_FILES})
add_library(${PROJECT_NAME} STATIC ${VAC_HEADER_FILES} ${VAC_SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ..) # So that downstream users can do e.g. #include <VAC/Layer.h>
target_compile_definitions(${PROJECT_NAME} PRIVATE _USE_MATH_DEFINES)
target_compile_definitions(${PROJECT_NAME} PRIVATE ACTION_MODIFIER_NAME="${ACTION_MODIFIER_NAME}")
target_compile_definitions(${PROJECT_NAME} PRIVATE ACTION_MODIFIER_NAME_SHORT="${ACTION_MODIFIER_NAME_SHORT}")

find_package(Qt5 COMPONENTS Core Gui Widgets OpenGL OpenGLExtensions Network REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC Qt5::Widgets Qt5::Core Qt5::Gui Qt5::OpenGL Qt5::OpenGLExtensions Qt5::Network)

find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC ${OPENGL_LIBRARIES})

find_package(Eigen3 QUIET)
if (Eigen3_FOUND)
    target_link_libraries(${PROJECT_NAME} PUBLIC Eigen3::Eigen)
else()
    target_include_directories(${PROJECT_NAME} PUBLIC ../Third)
endif()
