// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "TestOperators.h"

#include <OpenVAC/VAC.h>

#include <OpenVAC/Topology/KeyVertex.h>
#include <OpenVAC/Topology/KeyEdge.h>

#include <OpenVAC/Operators/OpMakeKeyVertex.h>
#include <OpenVAC/Operators/OpMakeKeyOpenEdge.h>

using namespace OpenVAC;

void TestOperators::opMakeKeyVertex()
{
    VAC vac;
    QVERIFY(vac.numCells() == 0);

    // Create a key vertex and test exhaustively
    OpMakeKeyVertex op(&vac, 42);
    QVERIFY(vac.numCells() == 0);

    QVERIFY(op.isValid());
    QVERIFY(vac.numCells() == 0);

    op.compute();
    QVERIFY(vac.numCells() == 0);

    op.apply();
    QVERIFY(vac.numCells() == 1);

    KeyVertexId keyVertexId = op.keyVertexId();
    std::vector<CellId> newCellsIds = op.newCells();
    QVERIFY(newCellsIds.size() == 1);
    QVERIFY(newCellsIds[0] == keyVertexId);

    KeyVertexHandle keyVertex = op.keyVertex();
    QVERIFY(keyVertex);
    QVERIFY(keyVertex == vac.cell(keyVertexId));
    QVERIFY(keyVertex->frame() == 42);

    // Example 1 of typical client code
    OpMakeKeyVertex(&vac, 11).apply();
    QVERIFY(vac.numCells() == 2);

    // Example 2 of typical client code
    KeyVertexHandle keyVertex2 = OpMakeKeyVertex(&vac, 12).apply().keyVertex();
    QVERIFY(vac.numCells() == 3);
    QVERIFY(keyVertex2->frame() == 12);

    // Example 3 of typical client code
    OpMakeKeyVertex op3 = OpMakeKeyVertex(&vac, 13).apply();
    KeyVertexHandle keyVertex3 = op3.keyVertex();
    QVERIFY(vac.numCells() == 4);
    QVERIFY(keyVertex3->frame() == 13);
}

void TestOperators::opMakeKeyOpenEdge()
{
    // Setup
    VAC vac;
    KeyVertexHandle keyVertex1 = OpMakeKeyVertex(&vac, 12).apply().keyVertex();
    KeyVertexHandle keyVertex2 = OpMakeKeyVertex(&vac, 12).apply().keyVertex();
    KeyVertexHandle keyVertex3 = OpMakeKeyVertex(&vac, 13).apply().keyVertex();
    QVERIFY(vac.numCells() == 3);

    // Create valid key edge
    KeyEdgeHandle keyEdge = OpMakeKeyOpenEdge(keyVertex1, keyVertex2).apply().keyEdge();
    QVERIFY(vac.numCells() == 4);
    QVERIFY(keyEdge);
    QVERIFY(keyEdge->frame() == 12);

    // Test invalid OpMakeKeyOpenEdge
    QVERIFY(!OpMakeKeyOpenEdge(keyVertex1, keyVertex3).isValid());
}
