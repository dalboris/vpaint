// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef VAC_SPLITMAP_H
#define VAC_SPLITMAP_H

#include "CellList.h"
#include <QPair>

namespace VectorAnimationComplex
{ 
    typedef QPair<Cell*, CellList> SplitMap;
}

#endif
