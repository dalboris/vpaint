// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

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
