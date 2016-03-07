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

#include <OpenVac/Operators/OpMakeKeyVertex.h>
#include <OpenVac/Operators/OpMakeKeyOpenEdge.h>

#include <vector>

namespace
{
class Geometry
{
public:
    typedef int Frame;
    class Manager {};
    class KeyVertex { double x; double y; };
    class KeyEdge { std::vector<double> curve; };
};
}

typedef OpenVac::Vac<Geometry> Vac;
typedef OpenVac::OpMakeKeyVertex<Geometry> OpMakeKeyVertex;
namespace Op = OpenVac::Operators;

void TestOperators::opMakeKeyVertex()
{
    // Using stack-allocated Vac
    Vac vac;
    QVERIFY(vac.numCells() == 0);

    // Create an OpMakeKeyVertex and test exhaustively
    OpMakeKeyVertex op(vac.ptr(), 42);
    QVERIFY(vac.numCells() == 0);

    QVERIFY(op.isValid());
    QVERIFY(vac.numCells() == 0);

    op.compute();
    QVERIFY(vac.numCells() == 0);

    op.apply();
    QVERIFY(vac.numCells() == 1);

    Vac::KeyVertexId keyVertexId = op.keyVertexId();
    std::vector<Vac::CellId> newCellsIds = op.newCells();
    QVERIFY(newCellsIds.size() == 1);
    QVERIFY(newCellsIds[0] == keyVertexId);

    Vac::KeyVertexHandle keyVertex = op.keyVertex();
    QVERIFY(keyVertex);
    QVERIFY(keyVertex == vac.cell(keyVertexId));
    QVERIFY(keyVertex->frame() == 42);

    // Example 1 of typical client code
    Op::makeKeyVertex(vac.ptr(), 42);
    QVERIFY(vac.numCells() == 2);

    // Example 2 of typical client code
    Vac::KeyVertexHandle keyVertex2 = Op::makeKeyVertex(vac.ptr(), 12);
    QVERIFY(vac.numCells() == 3);
    QVERIFY(keyVertex2->frame() == 12);

    // Example 3 of typical client code
    auto op3 = Op::MakeKeyVertex(vac.ptr(), 13);
    if (op3.isValid())
        op3.apply();
    auto keyVertex3 = op3.keyVertex();
    QVERIFY(vac.numCells() == 4);
    QVERIFY(keyVertex3->frame() == 13);
}

void TestOperators::opMakeKeyOpenEdge()
{
    // Setup (using shared_ptr-managed Vac)
    auto vac = Vac::make_shared();
    auto keyVertex1 = Op::makeKeyVertex(vac, 12);
    auto keyVertex2 = Op::makeKeyVertex(vac, 12);
    auto keyVertex3 = Op::makeKeyVertex(vac, 13);
    QVERIFY(vac->numCells() == 3);

    // Create valid key edge
    auto keyEdge = Op::makeKeyOpenEdge(keyVertex1, keyVertex2);
    QVERIFY(vac->numCells() == 4);
    QVERIFY(keyEdge);
    QVERIFY(keyEdge->frame() == 12);

    // Test invalid OpMakeKeyOpenEdge
    QVERIFY(!Op::MakeKeyOpenEdge(keyVertex1, keyVertex3).isValid());
}
