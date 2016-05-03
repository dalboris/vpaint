// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "Test.h"

#include <OpenVac/Core/Memory.h>

namespace
{
class Foo
{
public:
    Foo(int x) : x(x) {}
    int x;
};
}

typedef OpenVac::SharedPtr<Foo> FooSharedPtr;
typedef OpenVac::WeakPtr<Foo> FooPtr;

BEGIN_TESTS

void testMemory()
{
    FooPtr wp;
    QVERIFY(!wp);

    {
        FooSharedPtr sp = std::make_shared<Foo>(42);
        QCOMPARE(sp->x, 42);

        wp = sp;
        QVERIFY(!wp.expired());
        QCOMPARE(wp->x, 42);
    }

    QVERIFY(wp.expired());
}

END_TESTS
