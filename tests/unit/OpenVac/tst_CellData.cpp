// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "Test.h"

#include <OpenVac/Data/CellData.h>
#include <OpenVac/Data/KeyVertexData.h>
#include <OpenVac/Data/KeyEdgeData.h>
#include <OpenVac/Operators/Util/Ids.h>

#include <vector>

using namespace OpenVac;

BEGIN_TESTS

void createCellDataObjects()
{
    KeyVertexData<Ids> keyVertexData;
    KeyEdgeData<Ids> keyEdgeData;

    CellData<Ids> * cellData1 = new KeyVertexData<Ids>();
    CellData<Ids> * cellData2 = &keyVertexData;
    CellData<Ids> * cellData3 = new KeyEdgeData<Ids>();
    CellData<Ids> * cellData4 = &keyEdgeData;

    QVERIFY(keyVertexData.type() == CellType::KeyVertex);
    QVERIFY(cellData1->type()    == CellType::KeyVertex);
    QVERIFY(cellData2->type()    == CellType::KeyVertex);

    QVERIFY(keyEdgeData.type()   == CellType::KeyEdge);
    QVERIFY(cellData3->type()    == CellType::KeyEdge);
    QVERIFY(cellData4->type()    == CellType::KeyEdge);

    delete cellData1;
    delete cellData3;
}

END_TESTS
