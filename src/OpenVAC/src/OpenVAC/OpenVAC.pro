# Copyright (C) 2012-2016 The VPaint Developers.
# See the COPYRIGHT file at the top-level directory of this distribution
# and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
#
# This file is part of VPaint, a vector graphics editor. It is subject to the
# license terms and conditions in the LICENSE.MIT file found in the top-level
# directory of this distribution and at http://opensource.org/licenses/MIT

QT -= core gui

TARGET = ../../OpenVAC
TEMPLATE = lib
CONFIG += staticlib c++11

OPENVAC_SRC = ..
INCLUDEPATH += $$OPENVAC_SRC
DEPENDPATH += $$OPENVAC_SRC
unix: QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_ISYSTEM $$OPENVAC_SRC

SOURCES += \
    VAC.cpp \
    Core/Frame.cpp \
    Topology/CellType.cpp \
    Topology/Cell.cpp \
    Topology/KeyCell.cpp \
    Topology/VertexCell.cpp \
    Topology/EdgeCell.cpp \
    Topology/KeyVertex.cpp \
    Topology/KeyEdge.cpp \
    Operators/Operator.cpp \
    Operators/OpMakeKeyVertex.cpp \
    Geometry/GManager.cpp \
    DefaultGeometry/DGManager.cpp \
    Operators/CellDataConverter.cpp \
    Operators/OpMakeKeyOpenEdge.cpp

HEADERS += \
    VAC.h \
    Core/Memory.h \
    Core/Frame.h \
    Core/IdManager.h \
    Topology/TCellData/DefineCellData.h \
    Topology/TCellData/TCellData.h \
    Topology/TCellData/TKeyVertexData.h \
    Topology/TCellData/TKeyEdgeData.h \
    Topology/CellType.h \
    Topology/CellData.h \
    Topology/CellId.h \
    Topology/CellHandle.h \
    Topology/Cell.h \
    Topology/KeyCell.h \
    Topology/VertexCell.h \
    Topology/EdgeCell.h \
    Topology/KeyVertex.h \
    Topology/KeyEdge.h \
    Operators/Operator.h \
    Operators/OpMakeKeyVertex.h \
    Geometry/GManager.h \
    DefaultGeometry/DGManager.h \
    Operators/OpCellData.h \
    Operators/CellDataConverter.h \
    Operators/OpMakeKeyOpenEdge.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
