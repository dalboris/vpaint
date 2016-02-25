// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef TESTCELL_H
#define TESTCELL_H

#include "Test.h"

class TestCell: public QObject
{
    Q_OBJECT

private slots:
    void testAllocatingAndCasting();
};

REGISTER_TEST(TestCell)

#endif
