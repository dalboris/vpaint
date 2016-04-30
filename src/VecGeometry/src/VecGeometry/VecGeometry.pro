# Copyright (C) 2012-2016 The VPaint Developers.
# See the COPYRIGHT file at the top-level directory of this distribution
# and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
#
# This file is part of VPaint, a vector graphics editor. It is subject to the
# license terms and conditions in the LICENSE.MIT file found in the top-level
# directory of this distribution and at http://opensource.org/licenses/MIT

QT -= core gui

TARGET = ../../VecGeometry
TEMPLATE = lib
CONFIG += staticlib c++11

VECGEOMETRY_SRC = ..
INCLUDEPATH += $$VECGEOMETRY_SRC
DEPENDPATH += $$VECGEOMETRY_SRC
unix: QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_ISYSTEM $$VECGEOMETRY_SRC

###############################################################################
#                      SHIPPED EXTERNAL LIBRARIES

# Add shipped external libraries to includepath and dependpath
INCLUDEPATH += $$PWD/../Third/
DEPENDPATH += $$PWD/../Third/
!win32: QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_ISYSTEM $$PWD/../Third/



###############################################################################
#                            APP SOURCE FILES

SOURCES += \
    EdgeGeometry.cpp

HEADERS += \
    EdgeGeometry.h \
    EdgeGeometry.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
