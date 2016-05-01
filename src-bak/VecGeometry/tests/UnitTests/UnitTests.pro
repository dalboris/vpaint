# Copyright (C) 2012-2016 The VPaint Developers.
# See the COPYRIGHT file at the top-level directory of this distribution
# and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
#
# This file is part of VPaint, a vector graphics editor. It is subject to the
# license terms and conditions in the LICENSE.MIT file found in the top-level
# directory of this distribution and at http://opensource.org/licenses/MIT

QT -= core gui
QT += testlib
CONFIG += c++11

VECGEOMETRY_SRC       = $$PWD/../../src
VECGEOMETRY_OUT_UNIX  = $$OUT_PWD/../..
VECGEOMETRY_OUT_WIN32 = $$OUT_PWD/../../$$RELEASE_OR_DEBUG

INCLUDEPATH += $$VECGEOMETRY_SRC/
DEPENDPATH += $$VECGEOMETRY_SRC/
!win32: QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_ISYSTEM $$VECGEOMETRY_SRC/

win32 {
    LIBS += -L$$VECGEOMETRY_OUT_WIN32/
    win32-g++: PRE_TARGETDEPS += $$VECGEOMETRY_OUT_WIN32/libVecGeometry.a
    else:      PRE_TARGETDEPS += $$VECGEOMETRY_OUT_WIN32/VecGeometry.lib
}
else:unix {
    LIBS += -L$$VECGEOMETRY_OUT_UNIX/ -lVecGeometry
    PRE_TARGETDEPS += $$VECGEOMETRY_OUT_UNIX/libVecGeometry.a
}

SOURCES = \
    main.cpp

HEADERS += \
    Test.h
