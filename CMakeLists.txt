cmake_minimum_required(VERSION 3.11)

project(vpaint)

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/src/version.txt VPAINT_VERSION)
string(STRIP "${VPAINT_VERSION}" VPAINT_VERSION)
if(APPLE)
    set(ACTION_MODIFIER_NAME "Command")
    set(ACTION_MODIFIER_NAME_SHORT "Cmd")
else()
    set(ACTION_MODIFIER_NAME "Control")
    set(ACTION_MODIFIER_NAME_SHORT "Ctrl")
endif()

add_subdirectory(src/VAC)
add_subdirectory(src/Gui)
