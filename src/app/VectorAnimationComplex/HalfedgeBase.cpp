// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "HalfedgeBase.h"

#include <QTextStream>

#include "VAC.h"


namespace VectorAnimationComplex
{

namespace HalfedgeImpl
{

Cell * cellFromId(VAC * vac, int id)
{
    return ( vac ? vac->getCell(id) : 0 );
}

Cell * cellFromId(VAC * vac, Cell * cell)
{
    return ( cell ? cellFromId(vac,cell->id()) : 0 );
}

void save(EdgeCell * edge, bool side, QTextStream & out)
{
    if(edge)
        out << "(" << edge->id() << "," << side << ")";
    else
        out << "(" << -1 << "," << side << ")";
}

bool isValid(EdgeCell * edge)
{
    return edge;
}

bool isClosed(EdgeCell * edge)
{
    return ( isValid(edge) ? edge->isClosed() : false);
}

bool isEqual(EdgeCell * edge1, bool side1, EdgeCell * edge2, bool side2)
{
    return (edge1 == edge2) && (side1 == side2);
}

EdgeSample startSample(EdgeCell * edge, bool side, Time t)
{
    if(side)
        return edge->startSample(t);
    else
        return edge->endSample(t);
}

EdgeSample endSample(EdgeCell * edge, bool side, Time t)
{
    if(side)
        return edge->endSample(t);
    else
        return edge->startSample(t);
}

}

}
