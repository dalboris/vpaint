// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

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
