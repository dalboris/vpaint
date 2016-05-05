// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "Test.h"
#include "VPaint/Core/DataObject.h"


/****************** Basic DataObject definition ********************/

struct MyObjectData
{
    int x = 0;
};

class MyObject: public DataObject<MyObjectData>
{
};


/****************** Observer of DataObject ********************/

class Observer: public QObject
{
    Q_OBJECT

public:
    Observer(MyObject * obj) :
        obj_(obj)
    {
        connect(obj, &MyObject::changed, this, &Observer::onChanged_);
    }

    int x() { return x_; }

public slots:
    void onChanged_() { x_ = obj_->data().x; }

private:
    MyObject * obj_;
    int x_;
};


/************* DataObject counting constructions and destructions ***************/

class CountedObject: public DataObject<int>
{
public:
    CountedObject()  { ++numConstructed(); }
    ~CountedObject() { ++numDestructed(); }

    static int & numConstructed()
    {
        static int num = 0;
        return num;
    }

    static int & numDestructed()
    {
        static int num = 0;
        return num;
    }
};


/************************ BEGIN TESTS ****************************************/

BEGIN_TESTS

void createObject()
{
    MyObject obj;
}

void setData()
{
    const int INIT_VALUE = 0;
    const int VALUE      = 10;

    MyObject obj;
    QCOMPARE(obj.data().x, INIT_VALUE);

    MyObjectData data;
    data.x = VALUE;

    obj.setData(data);
    QCOMPARE(obj.data().x, VALUE);
}

void observeObject()
{
    const int VALUE = 10;

    MyObject obj;
    Observer observer(&obj);

    MyObjectData data;
    data.x = VALUE;
    obj.setData(data);

    QCOMPARE(observer.x(), VALUE);
}

void appendObjectPtrsToVector()
{
    const int NUM_OBJECTS_1 = 100;
    const int NUM_OBJECTS_2 = 10;

    {
        std::vector<DataObjectPtr<CountedObject>> objects;

        for (int i=0; i<NUM_OBJECTS_1; ++i)
        {
            objects.push_back(DataObjectPtr<CountedObject>());
        }

        QCOMPARE(CountedObject::numConstructed(), NUM_OBJECTS_1);
        QCOMPARE(CountedObject::numDestructed(),  0);

        objects.resize(NUM_OBJECTS_2);

        QCOMPARE(CountedObject::numConstructed(), NUM_OBJECTS_1);
        QCOMPARE(CountedObject::numDestructed(),  NUM_OBJECTS_1 - NUM_OBJECTS_2);

        objects.reserve(NUM_OBJECTS_1);

        QCOMPARE(CountedObject::numConstructed(), NUM_OBJECTS_1);
        QCOMPARE(CountedObject::numDestructed(),  NUM_OBJECTS_1 - NUM_OBJECTS_2);
    }

    QCOMPARE(CountedObject::numConstructed(), NUM_OBJECTS_1);
    QCOMPARE(CountedObject::numDestructed(),  NUM_OBJECTS_1);
}

END_TESTS
