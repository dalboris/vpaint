// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "CellObserver.h"
#include "Cell.h"

namespace VectorAnimationComplex
{

CellObserver::CellObserver()
{
}

void CellObserver::observe(Cell * cell)
{
    cell->addObserver(this);
}

void CellObserver::unobserve(Cell * cell)
{
    cell->removeObserver(this);
}

} // end namespace VectorAnimationComplex
