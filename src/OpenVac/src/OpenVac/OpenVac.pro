# Copyright (C) 2012-2016 The VPaint Developers.
# See the COPYRIGHT file at the top-level directory of this distribution
# and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
#
# This file is part of VPaint, a vector graphics editor. It is subject to the
# license terms and conditions in the LICENSE.MIT file found in the top-level
# directory of this distribution and at http://opensource.org/licenses/MIT

QT -= core gui

TARGET = ../../OpenVac
TEMPLATE = lib
CONFIG += staticlib c++11

OPENVAC_SRC = ..
INCLUDEPATH += $$OPENVAC_SRC
DEPENDPATH += $$OPENVAC_SRC
unix: QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_ISYSTEM $$OPENVAC_SRC

SOURCES +=

HEADERS += \
    Core/Memory.h \
    Core/IdManager.h \
    Topology/Cell.h \
    Topology/KeyCell.h \
    Topology/VertexCell.h \
    Topology/EdgeCell.h \
    Topology/KeyVertex.h \
    Topology/KeyEdge.h \
    Operators/Operator.h \
    Core/ForeachCellType.h \
    Vac.h \
    Geometry/DFrame.h \
    Core/CellType.h \
    Core/TypeTraits.h \
    Data/CellData.h \
    Data/KeyEdgeData.h \
    Data/KeyVertexData.h \
    Core/CellId.h \
    Data/UsingData.h \
    Geometry/UsingGeometry.h \
    Operators/CellDataCopier.h \
    Operators/MakeKeyVertex.h \
    Operators/MakeKeyOpenEdge.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
