// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef CELLOBSERVER_H
#define CELLOBSERVER_H

// Note: a cell observer can observe several cells
//       inheriting classes

namespace VectorAnimationComplex
{

class Cell;

class CellObserver
{
public:
    CellObserver();

    void observe(Cell * cell);
    void unobserve(Cell * cell);

    virtual void observedCellDeleted(Cell *) {}
};

} // end namespace VectorAnimationComplex

#endif // CELLOBSERVER_H
