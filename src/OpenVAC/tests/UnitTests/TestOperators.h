// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef TESTOPERATORS_H
#define TESTOPERATORS_H

#include "Test.h"

class TestOperators: public QObject
{
    Q_OBJECT

private slots:
    void opMakeKeyVertex();
    void opMakeKeyOpenEdge();
};

REGISTER_TEST(TestOperators)

#endif
