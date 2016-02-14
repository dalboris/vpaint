# Copyright (C) 2012-2016 The VPaint Developers.
# See the COPYRIGHT file at the top-level directory of this distribution
# and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
#
# This file is part of VPaint, a vector graphics editor. It is subject to the
# license terms and conditions in the LICENSE.MIT file found in the top-level
# directory of this distribution and at http://opensource.org/licenses/MIT

QT -= core gui
QT += testlib

OPENVAC_SRC       = $$PWD/../../src
OPENVAC_OUT_UNIX  = $$OUT_PWD/../..
OPENVAC_OUT_WIN32 = $$OUT_PWD/../../$$RELEASE_OR_DEBUG

INCLUDEPATH += $$OPENVAC_SRC/
DEPENDPATH += $$OPENVAC_SRC/
!win32: QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_ISYSTEM $$OPENVAC_SRC/

win32 {
    LIBS += -L$$OPENVAC_OUT_WIN32/
    win32-g++: PRE_TARGETDEPS += $$OPENVAC_OUT_WIN32/libOpenVAC.a
    else:      PRE_TARGETDEPS += $$OPENVAC_OUT_WIN32/OpenVAC.lib
}
else:unix {
    LIBS += -L$$OPENVAC_OUT_UNIX/ -lOpenVAC
    PRE_TARGETDEPS += $$OPENVAC_OUT_UNIX/libOpenVAC.a
}

SOURCES = \
    TestFrame.cpp
