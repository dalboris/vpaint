# Trick to have the source files of unit tests show up on Qt Creator, but don't
# do anything with them. Tests are compiled on the fly by the 'runtests' app.
requires(skip_project_on_purpose)

# Despite skipping this project, we still set up its configuration and include
# paths to have autocompletion on Qt Creator
TEMPLATE = lib
CONFIG += staticlib
QT += testlib widgets

# Add src/third/ and src/libs/ to INCLUDEPATH
INCLUDEPATH += ../../src/third/
INCLUDEPATH += ../../src/libs/

# Add tests/testlib/ to INCLUDEPATH
INCLUDEPATH += ../testlib/

# List of all unit tests
SOURCES += \
    OpenVac/tst_Cell.cpp \
    OpenVac/tst_CellData.cpp \
    OpenVac/tst_Frame.cpp \
    OpenVac/tst_Memory.cpp \
    OpenVac/tst_Operators.cpp
