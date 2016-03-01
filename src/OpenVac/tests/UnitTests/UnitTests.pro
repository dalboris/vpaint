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

OPENVAC_SRC       = $$PWD/../../src
OPENVAC_OUT_UNIX  = $$OUT_PWD/../..
OPENVAC_OUT_WIN32 = $$OUT_PWD/../../$$RELEASE_OR_DEBUG

INCLUDEPATH += $$OPENVAC_SRC/
DEPENDPATH += $$OPENVAC_SRC/
!win32: QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_ISYSTEM $$OPENVAC_SRC/

win32 {
    LIBS += -L$$OPENVAC_OUT_WIN32/
    win32-g++: PRE_TARGETDEPS += $$OPENVAC_OUT_WIN32/libOpenVac.a
    else:      PRE_TARGETDEPS += $$OPENVAC_OUT_WIN32/OpenVac.lib
}
else:unix {
    LIBS += -L$$OPENVAC_OUT_UNIX/ -lOpenVac
    PRE_TARGETDEPS += $$OPENVAC_OUT_UNIX/libOpenVac.a
}

SOURCES = \
    main.cpp \
    TestFrame.cpp \
    TestTCellData.cpp \
    TestMemory.cpp \
    TestCell.cpp \
    TestOperators.cpp

HEADERS += \
    Test.h \
    TestFrame.h \
    TestTCellData.h \
    TestMemory.h \
    TestCell.h \
    TestOperators.h
