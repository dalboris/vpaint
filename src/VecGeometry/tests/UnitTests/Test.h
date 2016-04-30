// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef TESTLIST_H
#define TESTLIST_H

#include <QtTest/QtTest>
#include <QObject>
#include <QList>

inline QList<QObject*> & testList()
{
    static QList<QObject*> list;
    return list;
}

inline void addTest(QObject * object)
{
    QList<QObject*> & list = testList();
    if (!list.contains(object))
    {
        list.append(object);
    }
}

template <class TestObject>
class RegisterTest
{
public:
    RegisterTest()
    {
        addTest(testObject());
    }

    inline TestObject * testObject()
    {
        static TestObject tc;
        return &tc;
    }
};

#define REGISTER_TEST(TestObject) \
    static RegisterTest<TestObject> TestObject##Instance;

#endif // TESTLIST_H
