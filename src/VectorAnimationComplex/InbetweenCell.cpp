// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

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

}
