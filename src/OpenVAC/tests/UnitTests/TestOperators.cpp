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
#include <OpenVAC/Operators/OpMakeKeyVertex.h>

using namespace OpenVAC;

void TestOperators::opMakeKeyVertex()
{
    VAC vac;
    QVERIFY(vac.numCells() == 0);

    // Create a key vertex
    OpMakeKeyVertex op(&vac, 42);
    QVERIFY(vac.numCells() == 0);

    QVERIFY(op.isValid());
    QVERIFY(vac.numCells() == 0);

    QVERIFY(op.compute());
    QVERIFY(vac.numCells() == 0);

    QVERIFY(op.apply());
    QVERIFY(vac.numCells() == 1);

    std::vector<CellId> newCells = op.newCells();
    QVERIFY(newCells.size() == 1);

    CellId id = newCells[0];
    KeyVertexHandle keyVertex = vac.cell(id);
    QVERIFY(keyVertex);
    QVERIFY(keyVertex->frame() == 42);

    // Create another key vertex
    OpMakeKeyVertex op2(&vac, 12);
    op2.apply();
    QVERIFY(vac.numCells() == 2);

    CellId id2 = op2.newCells()[0];
    KeyVertexHandle keyVertex2 = vac.cell(id2);
    QVERIFY(keyVertex2->frame() == 12);
}
