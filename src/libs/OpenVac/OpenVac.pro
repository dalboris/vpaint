# Copyright (C) 2012-2016 The VPaint Developers.
# See the COPYRIGHT file at the top-level directory of this distribution
# and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
#
# This file is part of VPaint, a vector graphics editor. It is subject to the
# license terms and conditions in the LICENSE.MIT file found in the top-level
# directory of this distribution and at http://opensource.org/licenses/MIT

TEMPLATE = lib
CONFIG -= qt

LIB_DEPENDS = \
    VecGeometry

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
    Core/CellType.h \
    Data/CellData.h \
    Data/KeyEdgeData.h \
    Data/KeyVertexData.h \
    Core/CellId.h \
    Operators/MakeKeyVertex.h \
    Operators/MakeKeyOpenEdge.h \
    Geometry.h \
    Geometry/Frame.h \
    Data/Util/CellDataCopier.h \
    Data/Util/CellDataCopierVisitor.h \
    Data/Util/CellRefTranslator.h \
    Operators/Util/HandleToIdTranslator.h \
    Operators/Util/IdToHandleTranslator.h \
    Operators/Util/HandlesToIdsCopier.h \
    Operators/Util/IdsToHandlesCopier.h \
    Data/Util/CellDataMutator.h \
    Data/Util/CellDataVisitor.h \
    Operators/Util/Ids.h \
    Topology/Util/Handles.h \
    Topology/Util/CellCast.h \
    Util/VacObserver.h

include($$OUT_PWD/.config.pri)
