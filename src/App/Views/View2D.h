// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef VIEW2D_H
#define VIEW2D_H

#include "Views/View.h"
#include "Views/View2DMouseEvent.h"

class View2DMouseEvent;

class View2D: public View
{
public:
    View2D(QWidget * parent = nullptr);

protected:
    View2DMouseEvent * makeMouseEvent();
};

#endif // VIEW2D_H
