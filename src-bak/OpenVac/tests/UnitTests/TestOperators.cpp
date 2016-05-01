// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "TestOperators.h"

#include <OpenVac/Vac.h>

#include <OpenVac/Topology/KeyVertex.h>
#include <OpenVac/Topology/KeyEdge.h>

#include <OpenVac/Operators/MakeKeyVertex.h>
#include <OpenVac/Operators/MakeKeyOpenEdge.h>

#include <vector>

using namespace OpenVac;

void TestOperators::opMakeKeyVertex()
{
    // Using stack-allocated Vac
    Vac vac;
    QVERIFY(vac.numCells() == 0);

    // Create an OpMakeKeyVertex and test exhaustively
    OpMakeKeyVertex op(42);
    QVERIFY(vac.numCells() == 0);

    op.compute(vac);
    QVERIFY(op.canBeApplied());
    QVERIFY(vac.numCells() == 0);

    op.apply(vac);
    QVERIFY(vac.numCells() == 1);

    KeyVertexId keyVertexId = op.keyVertexId();
    std::vector<CellId> newCellsIds = op.newCells();
    QVERIFY(newCellsIds.size() == 1);
    QVERIFY(newCellsIds[0] == keyVertexId);

    KeyVertexHandle keyVertex = vac.cell(op.keyVertexId());
    QVERIFY((bool)keyVertex);
    QVERIFY(keyVertex == vac.cell(keyVertexId));
    QVERIFY(keyVertex->frame() == 42);

    // Example 1 of typical client code
    Operators::makeKeyVertex(vac, 42);
    QVERIFY(vac.numCells() == 2);

    // Example 2 of typical client code
    KeyVertexHandle keyVertex2 = Operators::makeKeyVertex(vac, 12);
    QVERIFY(vac.numCells() == 3);
    QVERIFY((bool)keyVertex2);
    QVERIFY(keyVertex2->frame() == 12);

    // Example 3 of typical client code
    OpMakeKeyVertex op3(13);
    if (op3.compute(vac))
        op3.apply(vac);
    KeyVertexHandle keyVertex3 = vac.cell(op3.keyVertexId());
    QVERIFY(vac.numCells() == 4);
    QVERIFY((bool)keyVertex3);
    QVERIFY(keyVertex3->frame() == 13);
}

void TestOperators::opMakeKeyOpenEdge()
{
    // Setup (using a shared pointer to manage the Vac)
    auto vac = std::make_shared<Vac>();
    auto keyVertex1 = Operators::makeKeyVertex(*vac, 12);
    auto keyVertex2 = Operators::makeKeyVertex(*vac, 12);
    auto keyVertex3 = Operators::makeKeyVertex(*vac, 13);
    QVERIFY(vac->numCells() == 3);

    // Create valid key edge
    auto keyEdge1 = Operators::makeKeyOpenEdge(keyVertex1, keyVertex2);
    QVERIFY(vac->numCells() == 4);
    QVERIFY((bool)keyEdge1);
    QVERIFY(keyEdge1->frame() == 12);

    // Test invalid OpMakeKeyOpenEdge
    auto keyEdge2 = Operators::makeKeyOpenEdge(keyVertex1, keyVertex3);
    QVERIFY(vac->numCells() == 4);
    QVERIFY(!keyEdge2);

    // Test invalid OpMakeKeyOpenEdge
    OpMakeKeyOpenEdge op(keyVertex1->id(), keyVertex3->id());
    op.compute(*vac);
    QVERIFY(!op);
}
