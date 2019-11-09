// Copyright (C) 2012-2019 The VPaint Developers
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
