// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "TestFrame.h"

#include <OpenVac/Geometry/DFrame.h>

typedef OpenVac::DFrame Frame;

void TestFrame::createFrames()
{
    Frame f1;

    Frame f2(2.0);
    Frame f3(3);

    Frame f4 = Frame();
    Frame f5 = Frame(5.0);
    Frame f6 = Frame(6);

    Frame f7 = 7.0;
    Frame f8 = 8;

    QCOMPARE(f1.toDouble(), 0.0);
    QCOMPARE(f2.toDouble(), 2.0);
    QCOMPARE(f3.toDouble(), 3.0);
    QCOMPARE(f4.toDouble(), 0.0);
    QCOMPARE(f5.toDouble(), 5.0);
    QCOMPARE(f6.toDouble(), 6.0);
    QCOMPARE(f7.toDouble(), 7.0);
    QCOMPARE(f8.toDouble(), 8.0);
}

void TestFrame::compareFrames()
{
    Frame f1 = 1;

    Frame f1a(1);
    Frame f1b(1.0);
    Frame f1c = 1.0;
    Frame f1d; f1d = 1;
    Frame f1e; f1e = 1.0;
    Frame f1f = f1;

    QVERIFY(f1a == f1);
    QVERIFY(f1b == f1);
    QVERIFY(f1c == f1);
    QVERIFY(f1d == f1);
    QVERIFY(f1e == f1);
    QVERIFY(f1f == f1);

    QVERIFY(!(f1a != f1));
    QVERIFY(!(f1b != f1));
    QVERIFY(!(f1c != f1));
    QVERIFY(!(f1d != f1));
    QVERIFY(!(f1e != f1));
    QVERIFY(!(f1f != f1));

    Frame f2 = 2;

    QVERIFY(f1 != f2);
    QVERIFY(f1 < f2);
    QVERIFY(f1 <= f2);
    QVERIFY(!(f1 == f2));
    QVERIFY(!(f1 > f2));
    QVERIFY(!(f1 >= f2));

    Frame f15 = 1.5;

    QVERIFY(f1 != f15);
    QVERIFY(f1 < f15);
    QVERIFY(f1 <= f15);
    QVERIFY(!(f1 == f15));
    QVERIFY(!(f1 > f15));
    QVERIFY(!(f1 >= f15));

    Frame fJustBefore1 = 0.9999999999999999;
    Frame fJustAfter1  = 1.0000000000000002;

    QVERIFY(f1 == fJustBefore1);
    QVERIFY(f1 >= fJustBefore1);
    QVERIFY(f1 <= fJustBefore1);
    QVERIFY(!(f1 != fJustBefore1));
    QVERIFY(!(f1 < fJustBefore1));
    QVERIFY(!(f1 > fJustBefore1));

    QVERIFY(f1 == fJustAfter1);
    QVERIFY(f1 >= fJustAfter1);
    QVERIFY(f1 <= fJustAfter1);
    QVERIFY(!(f1 != fJustAfter1));
    QVERIFY(!(f1 < fJustAfter1));
    QVERIFY(!(f1 > fJustAfter1));

    Frame f0009 = 0.009;
    Frame f1001 = 1.001;

    QVERIFY(f1 != f0009);
    QVERIFY(f1 > f0009);
    QVERIFY(f1 >= f0009);
    QVERIFY(!(f1 == f0009));
    QVERIFY(!(f1 < f0009));
    QVERIFY(!(f1 <= f0009));

    QVERIFY(f1 != f1001);
    QVERIFY(f1 < f1001);
    QVERIFY(f1 <= f1001);
    QVERIFY(!(f1 == f1001));
    QVERIFY(!(f1 > f1001));
    QVERIFY(!(f1 >= f1001));
}

void TestFrame::roundFrames()
{
    Frame f1 = 1;
    Frame f12 = 1.2;
    Frame f18 = 1.8;
    Frame f2 = 2;

    Frame fJustBefore1 = 0.9999999999999999;
    Frame fJustAfter1  = 1.0000000000000002;

    Frame fm1 = -1;
    Frame fm12 = -1.2;
    Frame fm18 = -1.8;
    Frame fm2 = -2;

    QVERIFY(Frame::floor(f1)  == f1);
    QVERIFY(Frame::floor(f12) == f1);
    QVERIFY(Frame::floor(f18) == f1);
    QVERIFY(Frame::floor(f2)  == f2);
    QVERIFY(Frame::floor(fJustBefore1) == f1);
    QVERIFY(Frame::floor(fJustAfter1)  == f1);
    QVERIFY(Frame::floor(fm1)  == fm1);
    QVERIFY(Frame::floor(fm12) == fm2);
    QVERIFY(Frame::floor(fm18) == fm2);
    QVERIFY(Frame::floor(fm2)  == fm2);

    QVERIFY(Frame::ceil(f1)  == f1);
    QVERIFY(Frame::ceil(f12) == f2);
    QVERIFY(Frame::ceil(f18) == f2);
    QVERIFY(Frame::ceil(f2)  == f2);
    QVERIFY(Frame::ceil(fJustBefore1) == f1);
    QVERIFY(Frame::ceil(fJustAfter1)  == f1);
    QVERIFY(Frame::ceil(fm1)  == fm1);
    QVERIFY(Frame::ceil(fm12) == fm1);
    QVERIFY(Frame::ceil(fm18) == fm1);
    QVERIFY(Frame::ceil(fm2)  == fm2);

    QVERIFY(Frame::round(f1)  == f1);
    QVERIFY(Frame::round(f12) == f1);
    QVERIFY(Frame::round(f18) == f2);
    QVERIFY(Frame::round(f2)  == f2);
    QVERIFY(Frame::round(fJustBefore1) == f1);
    QVERIFY(Frame::round(fJustAfter1)  == f1);
    QVERIFY(Frame::round(fm1)  == fm1);
    QVERIFY(Frame::round(fm12) == fm1);
    QVERIFY(Frame::round(fm18) == fm2);
    QVERIFY(Frame::round(fm2)  == fm2);

    QVERIFY(f1.toInt()  == 1);
    QVERIFY(f12.toInt() == 1);
    QVERIFY(f18.toInt() == 1);
    QVERIFY(f2.toInt()  == 2);
    QVERIFY(fJustBefore1.toInt() == 1);
    QVERIFY(fJustAfter1.toInt()  == 1);
    QVERIFY(fm1.toInt()  == -1);
    QVERIFY(fm12.toInt() == -2);
    QVERIFY(fm18.toInt() == -2);
    QVERIFY(fm2.toInt()  == -2);

    QVERIFY(Frame::floor(f1).toDouble()  == 1.0);
    QVERIFY(Frame::floor(f12).toDouble() == 1.0);
    QVERIFY(Frame::floor(f18).toDouble() == 1.0);
    QVERIFY(Frame::floor(f2).toDouble()  == 2.0);
    QVERIFY(Frame::floor(fJustBefore1).toDouble() == 1.0);
    QVERIFY(Frame::floor(fJustAfter1).toDouble()  == 1.0);
    QVERIFY(Frame::floor(fm1).toDouble()  == -1.0);
    QVERIFY(Frame::floor(fm12).toDouble() == -2.0);
    QVERIFY(Frame::floor(fm18).toDouble() == -2.0);
    QVERIFY(Frame::floor(fm2).toDouble()  == -2.0);

    QVERIFY(f1.isInteger());
    QVERIFY(!f12.isInteger());
    QVERIFY(!f18.isInteger());
    QVERIFY(f2.isInteger());
    QVERIFY(fJustBefore1.isInteger());
    QVERIFY(fJustAfter1.isInteger());
    QVERIFY(fm1.isInteger());
    QVERIFY(!fm12.isInteger());
    QVERIFY(!fm18.isInteger());
    QVERIFY(fm2.isInteger());

    QVERIFY(!f1.isSubframe());
    QVERIFY(f12.isSubframe());
    QVERIFY(f18.isSubframe());
    QVERIFY(!f2.isSubframe());
    QVERIFY(!fJustBefore1.isSubframe());
    QVERIFY(!fJustAfter1.isSubframe());
    QVERIFY(!fm1.isSubframe());
    QVERIFY(fm12.isSubframe());
    QVERIFY(fm18.isSubframe());
    QVERIFY(!fm2.isSubframe());
}

void TestFrame::frameArithmetic()
{
    Frame f1 = 1;
    Frame f2 = 2;

    QVERIFY(f1 + f2 == 3);
    QVERIFY(f1 - f2 == -1);
    QVERIFY(f1 + 4 == 5);
    QVERIFY(4 + f1 == 5);
    QVERIFY(0.5 * (f1 + f2) == 1.5);
    QVERIFY((f1 + f2) * 0.5 == 1.5);
    QVERIFY((f1 + f2) / 2.0 == 1.5);
    QVERIFY((f1 + f2) / 2   == 1.5);

    f1 += f2;
    QVERIFY(f1 == 3);

    f1 -= f1;
    QVERIFY(f1 == 0);
    QVERIFY(f1++ == 0);
    QVERIFY(++f1 == 2);
    QVERIFY(f1-- == 2);
    QVERIFY(--f1 == 0);

    f2 *= 5;
    QVERIFY(f2 == 10);

    f2 /= 20;
    QVERIFY(f2 == 0.5);

    Frame f3, f4;
    int n = 10;
    double dd = 1.0 / n;
    Frame df = 1.0 / n;
    for (int i=0; i<n; ++i) {
        f3 += dd;
        f4 += df;
    }
    QVERIFY(f3 == 1);
    QVERIFY(f4 == 1);
}

void TestFrame::frameToTime()
{
    int fps = 24;

    QVERIFY(qFuzzyCompare(Frame().toSeconds(fps), 0.0));
    QVERIFY(qFuzzyCompare(Frame(1).toSeconds(fps), 1.0 / 24));
    QVERIFY(qFuzzyCompare(Frame(12).toSeconds(fps), 0.5));
    QVERIFY(qFuzzyCompare(Frame(24).toSeconds(fps), 1.0));
    QVERIFY(qFuzzyCompare(Frame(48).toSeconds(fps), 2.0));

    QVERIFY(Frame::fromSeconds(0.0, fps) == 0);
    QVERIFY(Frame::fromSeconds(0.5, fps) == 12);
    QVERIFY(Frame::fromSeconds(1.0, fps) == 24);
    QVERIFY(Frame::fromSeconds(2.0, fps) == 48);
    QVERIFY(Frame::fromSeconds(-1.0, fps) == -24);
}
