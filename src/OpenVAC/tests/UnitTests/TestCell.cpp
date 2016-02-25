// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "TestCell.h"

#include <OpenVAC/Topology/KeyVertex.h>
#include <OpenVAC/Topology/KeyEdge.h>

using namespace OpenVAC;

void TestCell::testAllocatingAndCasting()
{
    // Some dummy data
    VAC * vac = nullptr;
    CellId id1 = 1;
    CellId id2 = 2;
    KeyVertexData keyVertexData; keyVertexData.frame = 42;
    KeyEdgeData   keyEdgeData;   keyEdgeData.frame = 43;

    // Empty handles
    {
        CellHandle cell;
        KeyCellHandle keyCell;
        VertexCellHandle vertexCell;
        EdgeCellHandle edgeCell;
        KeyVertexHandle keyVertex;
        KeyEdgeHandle keyEdge;
        QVERIFY(!cell);
        QVERIFY(!keyCell);
        QVERIFY(!vertexCell);
        QVERIFY(!edgeCell);
        QVERIFY(!keyVertex);
        QVERIFY(!keyEdge);
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

        QVERIFY(scell1->type() == CellType::KeyVertex);
        QVERIFY(scell2->type() == CellType::KeyEdge);

        QVERIFY(scell1->data().toKeyVertexData()->frame == keyVertexData.frame);
        QVERIFY(scell2->data().toKeyEdgeData()->frame == keyEdgeData.frame);

        keyVertexData.frame = 44;
        QVERIFY(scell1->data().toKeyVertexData()->frame != keyVertexData.frame);
    }

    // Get cell handles from upcasted cell shared pointers
    {
        CellSharedPtr scell1 = std::make_shared<KeyVertex>(vac, id1, keyVertexData);
        CellSharedPtr scell2 = std::make_shared<KeyEdge>(vac, id2, keyEdgeData);

        CellHandle cell1 = scell1;
        CellHandle cell2 = scell2;
        QVERIFY(cell1);
        QVERIFY(cell2);
        QVERIFY(cell1->vac() == vac);
        QVERIFY(cell2->vac() == vac);
        QVERIFY(cell1->id() == id1);
        QVERIFY(cell2->id() == id2);
        QVERIFY(cell1->type() == CellType::KeyVertex);
        QVERIFY(cell2->type() == CellType::KeyEdge);

        KeyCellHandle keyCell1 = scell1;
        KeyCellHandle keyCell2 = scell2;
        QVERIFY(keyCell1);
        QVERIFY(keyCell2);
        QVERIFY(keyCell1->frame() == keyVertexData.frame);
        QVERIFY(keyCell2->frame() == keyEdgeData.frame);

        VertexCellHandle vertexCell1 = scell1;
        VertexCellHandle vertexCell2 = scell2;
        QVERIFY(vertexCell1);
        QVERIFY(!vertexCell2);

        EdgeCellHandle edgeCell1 = scell1;
        EdgeCellHandle edgeCell2 = scell2;
        QVERIFY(!edgeCell1);
        QVERIFY(edgeCell2);

        KeyVertexHandle keyVertex1 = scell1;
        KeyVertexHandle keyVertex2 = scell2;
        QVERIFY(keyVertex1);
        QVERIFY(!keyVertex2);

        KeyEdgeHandle keyEdge1 = scell1;
        KeyEdgeHandle keyEdge2 = scell2;
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

        KeyVertexHandle keyVertex = scell1;
        KeyEdgeHandle keyEdge = scell2;
        QVERIFY(keyVertex);
        QVERIFY(keyEdge);

        CellHandle cell1 = keyVertex;
        CellHandle cell2 = keyEdge;
        QVERIFY(cell1);
        QVERIFY(cell2);
        QVERIFY(cell1->vac() == vac);
        QVERIFY(cell2->vac() == vac);
        QVERIFY(cell1->id() == id1);
        QVERIFY(cell2->id() == id2);
        QVERIFY(cell1->type() == CellType::KeyVertex);
        QVERIFY(cell2->type() == CellType::KeyEdge);

        KeyCellHandle keyCell1 = keyVertex;
        KeyCellHandle keyCell2 = keyEdge;
        QVERIFY(keyCell1);
        QVERIFY(keyCell2);
        QVERIFY(keyCell1->frame() == keyVertexData.frame);
        QVERIFY(keyCell2->frame() == keyEdgeData.frame);

        VertexCellHandle vertexCell1 = keyVertex;
        VertexCellHandle vertexCell2 = keyEdge;
        QVERIFY(vertexCell1);
        QVERIFY(!vertexCell2);

        EdgeCellHandle edgeCell1 = keyVertex;
        EdgeCellHandle edgeCell2 = keyEdge;
        QVERIFY(!edgeCell1);
        QVERIFY(edgeCell2);

        KeyVertexHandle keyVertex1 = keyVertex;
        KeyVertexHandle keyVertex2 = keyEdge;
        QVERIFY(keyVertex1);
        QVERIFY(!keyVertex2);

        KeyEdgeHandle keyEdge1 = keyVertex;
        KeyEdgeHandle keyEdge2 = keyEdge;
        QVERIFY(!keyEdge1);
        QVERIFY(keyEdge2);

        CellHandle cell3 = keyCell1;
        CellHandle cell4 = keyCell2;
        CellHandle cell5 = vertexCell1;
        CellHandle cell6 = vertexCell2;
        CellHandle cell7 = edgeCell1;
        CellHandle cell8 = edgeCell2;
        QVERIFY(cell3);
        QVERIFY(cell4);
        QVERIFY(cell5);
        QVERIFY(!cell6);
        QVERIFY(!cell7);
        QVERIFY(cell8);

        KeyEdgeHandle keyEdge3 = keyVertex2;
        QVERIFY(!keyEdge3);
    }

    // Downcasting cell handles
    {
        CellSharedPtr scell1 = std::make_shared<KeyVertex>(vac, id1, keyVertexData);
        CellSharedPtr scell2 = std::make_shared<KeyEdge>(vac, id2, keyEdgeData);
        CellHandle cell1 = scell1;
        CellHandle cell2 = scell2;

        CellHandle cell3 = cell1;
        CellHandle cell4 = cell2;
        QVERIFY(cell3);
        QVERIFY(cell4);

        KeyCellHandle keyCell1 = cell1;
        KeyCellHandle keyCell2 = cell2;
        QVERIFY(keyCell1);
        QVERIFY(keyCell2);
        QVERIFY(keyCell1->frame() == keyVertexData.frame);
        QVERIFY(keyCell2->frame() == keyEdgeData.frame);

        VertexCellHandle vertexCell1 = cell1;
        VertexCellHandle vertexCell2 = cell2;
        QVERIFY(vertexCell1);
        QVERIFY(!vertexCell2);

        EdgeCellHandle edgeCell1 = cell1;
        EdgeCellHandle edgeCell2 = cell2;
        QVERIFY(!edgeCell1);
        QVERIFY(edgeCell2);

        KeyVertexHandle keyVertex1 = cell1;
        KeyVertexHandle keyVertex2 = cell2;
        KeyVertexHandle keyVertex3 = keyCell1;
        KeyVertexHandle keyVertex4 = keyCell2;
        KeyVertexHandle keyVertex5 = vertexCell1;
        KeyVertexHandle keyVertex6 = vertexCell2;
        KeyVertexHandle keyVertex7 = edgeCell1;
        KeyVertexHandle keyVertex8 = edgeCell2;
        QVERIFY(keyVertex1);
        QVERIFY(!keyVertex2);
        QVERIFY(keyVertex3);
        QVERIFY(!keyVertex4);
        QVERIFY(keyVertex5);
        QVERIFY(!keyVertex6);
        QVERIFY(!keyVertex7);
        QVERIFY(!keyVertex8);

        KeyEdgeHandle keyEdge1 = cell1;
        KeyEdgeHandle keyEdge2 = cell2;
        KeyEdgeHandle keyEdge3 = keyCell1;
        KeyEdgeHandle keyEdge4 = keyCell2;
        KeyEdgeHandle keyEdge5 = vertexCell1;
        KeyEdgeHandle keyEdge6 = vertexCell2;
        KeyEdgeHandle keyEdge7 = edgeCell1;
        KeyEdgeHandle keyEdge8 = edgeCell2;
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
