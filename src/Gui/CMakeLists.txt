project(VPaint)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(SOURCE_FILES
    Application.cpp
    UpdateCheck.cpp
    UpdateCheckDialog.cpp
    main.cpp
    Gui.rc
    resources.qrc
)
set(HEADER_FILES
    Application.h
    UpdateCheck.h
    UpdateCheckDialog.h
)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_compile_definitions(${PROJECT_NAME} PRIVATE _USE_MATH_DEFINES)
target_compile_definitions(${PROJECT_NAME} PRIVATE APP_VERSION="${VPAINT_VERSION}")
target_compile_definitions(${PROJECT_NAME} PRIVATE ACTION_MODIFIER_NAME="${ACTION_MODIFIER_NAME}")
target_compile_definitions(${PROJECT_NAME} PRIVATE ACTION_MODIFIER_NAME_SHORT="${ACTION_MODIFIER_NAME_SHORT}")

target_link_libraries(${PROJECT_NAME} PRIVATE VAC)
