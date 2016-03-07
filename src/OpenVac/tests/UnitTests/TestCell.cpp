// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "TestCell.h"

#include <OpenVac/Vac.h>
#include <OpenVac/Topology/KeyVertex.h>
#include <OpenVac/Topology/KeyEdge.h>

#include <map>

namespace
{
class Geometry
{
public:
    typedef int Frame;
    class Manager {};
    class KeyVertex {};
    class KeyEdge {};
};
}

typedef OpenVac::Vac<Geometry> Vac;
typedef OpenVac::KeyVertex<Geometry> KeyVertex;
typedef OpenVac::KeyEdge<Geometry> KeyEdge;
typedef OpenVac::CellSharedPtr<Geometry> CellSharedPtr;

void TestCell::testAllocatingAndCasting()
{
    // Some dummy data
    Vac::Ptr vac; // null vac pointer
    Vac::CellId id1 = 1;
    Vac::CellId id2 = 2;
    Vac::KeyVertexData keyVertexData; keyVertexData.frame = 42;
    Vac::KeyEdgeData   keyEdgeData;   keyEdgeData.frame = 43;

    // Empty handles
    {
        Vac::CellHandle cell;
        Vac::KeyCellHandle keyCell;
        Vac::VertexCellHandle vertexCell;
        Vac::EdgeCellHandle edgeCell;
        Vac::KeyVertexHandle keyVertex;
        Vac::KeyEdgeHandle keyEdge;
        QVERIFY(!cell);
        QVERIFY(!keyCell);
        QVERIFY(!vertexCell);
        QVERIFY(!edgeCell);
        QVERIFY(!keyVertex);
        QVERIFY(!keyEdge);
    }

    // cell shared pointers to derived type
    {
        OpenVac::SharedPtr<KeyVertex> spv = std::make_shared<KeyVertex>(vac, id1, keyVertexData);
        OpenVac::SharedPtr<KeyEdge> spe = std::make_shared<KeyEdge>(vac, id2, keyEdgeData);

        QVERIFY(spv->type() == OpenVac::CellType::KeyVertex);
        QVERIFY(spe->type() == OpenVac::CellType::KeyEdge);
    }

    // cell shared pointers to base type
    {
        CellSharedPtr spv = std::make_shared<KeyVertex>(vac, id1, keyVertexData);
        CellSharedPtr spe = std::make_shared<KeyEdge>(vac, id2, keyEdgeData);

        QVERIFY(spv->type() == OpenVac::CellType::KeyVertex);
        QVERIFY(spe->type() == OpenVac::CellType::KeyEdge);
    }

    // cell shared pointers to base type stored in map
    {
        std::map<int, CellSharedPtr> map;
        {
            CellSharedPtr spv = std::make_shared<KeyVertex>(vac, id1, keyVertexData);
            CellSharedPtr spe = std::make_shared<KeyEdge>(vac, id2, keyEdgeData);

            map[1] = spv;
            map[2] = spe;

            QVERIFY(spv->type() == OpenVac::CellType::KeyVertex);
            QVERIFY(spe->type() == OpenVac::CellType::KeyEdge);
        }

        CellSharedPtr spv = map[1];
        CellSharedPtr spe = map[2];

        QVERIFY(spv->type() == OpenVac::CellType::KeyVertex);
        QVERIFY(spe->type() == OpenVac::CellType::KeyEdge);
    }

    // Allocating cell objects and managing them with upcasted cell shared pointers
    {
        CellSharedPtr scell1 = std::make_shared<KeyVertex>(vac, id1, keyVertexData);
        CellSharedPtr scell2 = std::make_shared<KeyEdge>(vac, id2, keyEdgeData);

        QVERIFY((bool)scell1);
        QVERIFY((bool)scell2);

        QVERIFY(scell1->vac() == vac);
        QVERIFY(scell2->vac() == vac);

        QVERIFY(scell1->id() == id1);
        QVERIFY(scell2->id() == id2);

        QVERIFY(scell1->type() == OpenVac::CellType::KeyVertex);
        QVERIFY(scell2->type() == OpenVac::CellType::KeyEdge);
    }

    // Get cell handles from upcasted cell shared pointers
    {
        CellSharedPtr scell1 = std::make_shared<KeyVertex>(vac, id1, keyVertexData);
        CellSharedPtr scell2 = std::make_shared<KeyEdge>(vac, id2, keyEdgeData);

        Vac::CellHandle cell1 = scell1;
        Vac::CellHandle cell2 = scell2;
        QVERIFY(cell1);
        QVERIFY(cell2);
        QVERIFY(cell1->vac() == vac);
        QVERIFY(cell2->vac() == vac);
        QVERIFY(cell1->id() == id1);
        QVERIFY(cell2->id() == id2);
        QVERIFY(cell1->type() == OpenVac::CellType::KeyVertex);
        QVERIFY(cell2->type() == OpenVac::CellType::KeyEdge);

        Vac::KeyCellHandle keyCell1 = scell1;
        Vac::KeyCellHandle keyCell2 = scell2;
        QVERIFY(keyCell1);
        QVERIFY(keyCell2);
        QVERIFY(keyCell1->frame() == keyVertexData.frame);
        QVERIFY(keyCell2->frame() == keyEdgeData.frame);

        Vac::VertexCellHandle vertexCell1 = scell1;
        Vac::VertexCellHandle vertexCell2 = scell2;
        QVERIFY(vertexCell1);
        QVERIFY(!vertexCell2);

        Vac::EdgeCellHandle edgeCell1 = scell1;
        Vac::EdgeCellHandle edgeCell2 = scell2;
        QVERIFY(!edgeCell1);
        QVERIFY(edgeCell2);

        Vac::KeyVertexHandle keyVertex1 = scell1;
        Vac::KeyVertexHandle keyVertex2 = scell2;
        QVERIFY(keyVertex1);
        QVERIFY(!keyVertex2);

        Vac::KeyEdgeHandle keyEdge1 = scell1;
        Vac::KeyEdgeHandle keyEdge2 = scell2;
        QVERIFY(!keyEdge1);
        QVERIFY(keyEdge2);

        scell1.reset();
        QVERIFY(!cell1);
        QVERIFY(!keyCell1);
        QVERIFY(!vertexCell1);
        QVERIFY(!edgeCell1);
        QVERIFY(!keyVertex1);
        QVERIFY(!keyEdge1);

        scell2.reset();
        QVERIFY(!cell2);
        QVERIFY(!keyCell2);
        QVERIFY(!vertexCell2);
        QVERIFY(!edgeCell2);
        QVERIFY(!keyVertex2);
        QVERIFY(!keyEdge2);
    }

    // Upcasting cell handles
    {
        CellSharedPtr scell1 = std::make_shared<KeyVertex>(vac, id1, keyVertexData);
        CellSharedPtr scell2 = std::make_shared<KeyEdge>(vac, id2, keyEdgeData);

        Vac::KeyVertexHandle keyVertex = scell1;
        Vac::KeyEdgeHandle keyEdge = scell2;
        QVERIFY(keyVertex);
        QVERIFY(keyEdge);

        Vac::CellHandle cell1 = keyVertex;
        Vac::CellHandle cell2 = keyEdge;
        QVERIFY(cell1);
        QVERIFY(cell2);
        QVERIFY(cell1->vac() == vac);
        QVERIFY(cell2->vac() == vac);
        QVERIFY(cell1->id() == id1);
        QVERIFY(cell2->id() == id2);
        QVERIFY(cell1->type() == OpenVac::CellType::KeyVertex);
        QVERIFY(cell2->type() == OpenVac::CellType::KeyEdge);

        Vac::KeyCellHandle keyCell1 = keyVertex;
        Vac::KeyCellHandle keyCell2 = keyEdge;
        QVERIFY(keyCell1);
        QVERIFY(keyCell2);
        QVERIFY(keyCell1->frame() == keyVertexData.frame);
        QVERIFY(keyCell2->frame() == keyEdgeData.frame);

        Vac::VertexCellHandle vertexCell1 = keyVertex;
        Vac::VertexCellHandle vertexCell2 = keyEdge;
        QVERIFY(vertexCell1);
        QVERIFY(!vertexCell2);

        Vac::EdgeCellHandle edgeCell1 = keyVertex;
        Vac::EdgeCellHandle edgeCell2 = keyEdge;
        QVERIFY(!edgeCell1);
        QVERIFY(edgeCell2);

        Vac::KeyVertexHandle keyVertex1 = keyVertex;
        Vac::KeyVertexHandle keyVertex2 = keyEdge;
        QVERIFY(keyVertex1);
        QVERIFY(!keyVertex2);

        Vac::KeyEdgeHandle keyEdge1 = keyVertex;
        Vac::KeyEdgeHandle keyEdge2 = keyEdge;
        QVERIFY(!keyEdge1);
        QVERIFY(keyEdge2);

        Vac::CellHandle cell3 = keyCell1;
        Vac::CellHandle cell4 = keyCell2;
        Vac::CellHandle cell5 = vertexCell1;
        Vac::CellHandle cell6 = vertexCell2;
        Vac::CellHandle cell7 = edgeCell1;
        Vac::CellHandle cell8 = edgeCell2;
        QVERIFY(cell3);
        QVERIFY(cell4);
        QVERIFY(cell5);
        QVERIFY(!cell6);
        QVERIFY(!cell7);
        QVERIFY(cell8);

        Vac::KeyEdgeHandle keyEdge3 = keyVertex2;
        QVERIFY(!keyEdge3);
    }

    // Downcasting cell handles
    {
        CellSharedPtr scell1 = std::make_shared<KeyVertex>(vac, id1, keyVertexData);
        CellSharedPtr scell2 = std::make_shared<KeyEdge>(vac, id2, keyEdgeData);
        Vac::CellHandle cell1 = scell1;
        Vac::CellHandle cell2 = scell2;

        Vac::CellHandle cell3 = cell1;
        Vac::CellHandle cell4 = cell2;
        QVERIFY(cell3);
        QVERIFY(cell4);

        Vac::KeyCellHandle keyCell1 = cell1;
        Vac::KeyCellHandle keyCell2 = cell2;
        QVERIFY(keyCell1);
        QVERIFY(keyCell2);
        QVERIFY(keyCell1->frame() == keyVertexData.frame);
        QVERIFY(keyCell2->frame() == keyEdgeData.frame);

        Vac::VertexCellHandle vertexCell1 = cell1;
        Vac::VertexCellHandle vertexCell2 = cell2;
        QVERIFY(vertexCell1);
        QVERIFY(!vertexCell2);

        Vac::EdgeCellHandle edgeCell1 = cell1;
        Vac::EdgeCellHandle edgeCell2 = cell2;
        QVERIFY(!edgeCell1);
        QVERIFY(edgeCell2);

        Vac::KeyVertexHandle keyVertex1 = cell1;
        Vac::KeyVertexHandle keyVertex2 = cell2;
        Vac::KeyVertexHandle keyVertex3 = keyCell1;
        Vac::KeyVertexHandle keyVertex4 = keyCell2;
        Vac::KeyVertexHandle keyVertex5 = vertexCell1;
        Vac::KeyVertexHandle keyVertex6 = vertexCell2;
        Vac::KeyVertexHandle keyVertex7 = edgeCell1;
        Vac::KeyVertexHandle keyVertex8 = edgeCell2;
        QVERIFY(keyVertex1);
        QVERIFY(!keyVertex2);
        QVERIFY(keyVertex3);
        QVERIFY(!keyVertex4);
        QVERIFY(keyVertex5);
        QVERIFY(!keyVertex6);
        QVERIFY(!keyVertex7);
        QVERIFY(!keyVertex8);

        Vac::KeyEdgeHandle keyEdge1 = cell1;
        Vac::KeyEdgeHandle keyEdge2 = cell2;
        Vac::KeyEdgeHandle keyEdge3 = keyCell1;
        Vac::KeyEdgeHandle keyEdge4 = keyCell2;
        Vac::KeyEdgeHandle keyEdge5 = vertexCell1;
        Vac::KeyEdgeHandle keyEdge6 = vertexCell2;
        Vac::KeyEdgeHandle keyEdge7 = edgeCell1;
        Vac::KeyEdgeHandle keyEdge8 = edgeCell2;
        QVERIFY(!keyEdge1);
        QVERIFY(keyEdge2);
        QVERIFY(!keyEdge3);
        QVERIFY(keyEdge4);
        QVERIFY(!keyEdge5);
        QVERIFY(!keyEdge6);
        QVERIFY(!keyEdge7);
        QVERIFY(keyEdge8);
    }
}


