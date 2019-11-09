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

#include "InbetweenCell.h"
#include "KeyCell.h"
#include "VAC.h"

#include <QTextStream>
#include "../SaveAndLoad.h"
#include <QtDebug>

namespace VectorAnimationComplex
{

InbetweenCell::InbetweenCell(VAC * vac) :
    Cell(vac)
{
}

InbetweenCell::InbetweenCell(InbetweenCell * other) :
    Cell(other)
{
}

void InbetweenCell::remapPointers(VAC * /*newVAC*/)
{
}

InbetweenCell::~InbetweenCell()
{
}

InbetweenCell::InbetweenCell(VAC * vac, QTextStream & in) :
    Cell(vac, in)
{
}

void InbetweenCell::read2ndPass()
{
}

void InbetweenCell::save_(QTextStream & /*out*/)
{
}

InbetweenCell::InbetweenCell(VAC * vac, XmlStreamReader & xml) :
    Cell(vac, xml)
{
}

void InbetweenCell::write_(XmlStreamWriter & /*xml*/) const
{
}

Time InbetweenCell::beforeTime() const
{
    // Note: Assume there is at least one before object and
    //       that all before objects share the same time.
    KeyCellSet before = beforeCells();
    if(before.isEmpty())
        return Time();
    else
        return (*before.begin())->time();
}

Time InbetweenCell::afterTime() const
{
    // Note: Assume there is at least one after object and
    //       that all after objects share the same time.
    KeyCellSet after = afterCells();
    if(after.isEmpty())
        return Time();
    else
        return (*after.begin())->time();
}

bool InbetweenCell::exists(Time t) const
{
    return (beforeTime() < t) && (t < afterTime());
}

bool InbetweenCell::isBefore(Time t) const
{
    return afterTime() <= t;
}

bool InbetweenCell::isAfter(Time t) const
{
    return beforeTime() >= t;
}

bool InbetweenCell::isAt(Time /*t*/) const
{
    return false;
}

bool InbetweenCell::checkAnimated_() const
{
    // todo
    return true;
}

BoundingBox InbetweenCell::boundingBox() const
{
    // Get before and after frame
    int beforeFrame = beforeTime().frame();
    int afterFrame = beforeTime().frame();

    // Take the union of all bounding boxes in the middle
    // of each frame.
    //
    // Examples:
    // 1) beforeFrame = 12; afterFrame = 13
    //    => returns boundingBox(Time(12.5))
    //
    // 2) beforeFrame = 12; afterFrame = 14
    //    => returns boundingBox(Time(12.5)) UNION boundingBox(Time(13.5))
    BoundingBox res;
    for (double t = beforeFrame + 0.5; t < afterFrame; t += 1.0)
    {
        res.unite(boundingBox(Time(t)));
    }
    return res;
}

BoundingBox InbetweenCell::outlineBoundingBox() const
{
    // Same as above
    int beforeFrame = beforeTime().frame();
    int afterFrame = beforeTime().frame();
    BoundingBox res;
    for (double t = beforeFrame + 0.5; t < afterFrame; t += 1.0)
    {
        res.unite(outlineBoundingBox(Time(t)));
    }
    return res;
}

}
