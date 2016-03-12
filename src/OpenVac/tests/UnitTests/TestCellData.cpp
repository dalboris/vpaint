// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "TestCellData.h"

#include <OpenVac/Core/ForeachCellType.h>
#include <OpenVac/Data/CellData.h>
#include <OpenVac/Data/KeyVertexData.h>
#include <OpenVac/Data/KeyEdgeData.h>

#include <vector>

namespace
{
class Geometry
{
public:
    typedef int Frame;
    struct KeyVertexGeometry { double position; };
    struct KeyEdgeGeometry { std::vector<double> curve; };
};

class UintAsRefs
{
public:
    using KeyVertexRef = unsigned int;
    using KeyEdgeRef   = unsigned int;
};
}

#define DECLARE_MY_CELL_DATA(CellType) \
    using My##CellType##Data = OpenVac::CellType##Data<UintAsRefs, Geometry>;

OPENVAC_FOREACH_CELL_DATA_TYPE(DECLARE_MY_CELL_DATA)

void TestCellData::createCellDataObjects()
{
    MyKeyVertexData keyVertexData;
    MyKeyEdgeData keyEdgeData;

    MyCellData * cellData1 = new MyKeyVertexData();
    MyCellData * cellData2 = &keyVertexData;
    MyCellData * cellData3 = new MyKeyEdgeData();
    MyCellData * cellData4 = &keyEdgeData;

    QVERIFY(keyVertexData.type() == OpenVac::CellType::KeyVertex);
    QVERIFY(cellData1->type()    == OpenVac::CellType::KeyVertex);
    QVERIFY(cellData2->type()    == OpenVac::CellType::KeyVertex);

    QVERIFY(keyEdgeData.type()   == OpenVac::CellType::KeyEdge);
    QVERIFY(cellData3->type()    == OpenVac::CellType::KeyEdge);
    QVERIFY(cellData4->type()    == OpenVac::CellType::KeyEdge);

    delete cellData1;
    delete cellData3;
}
