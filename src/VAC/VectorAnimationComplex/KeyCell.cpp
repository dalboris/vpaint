// Copyright (C) 2012-2019 The VPaint Developers.
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

#include <limits>
#include <QtDebug>
#include "../OpenGL.h"

#include "../XmlStreamReader.h"
#include "../XmlStreamWriter.h"

namespace VectorAnimationComplex
{

KeyCell::KeyCell(VAC * vac, Time time) :
    Cell(vac),
    time_(time)
{
}

KeyCell::KeyCell(KeyCell * other) :
    Cell(other)
{
    time_ = other->time_;
}

Time KeyCell::temporalDragMinTime() const
{
    //Time res(std::numeric_limits<int>::min()); // Caution: can overflow if compute deltas
    Time res(-1000); // TODO

    InbetweenCellSet beforeStar = temporalStarBefore();
    for(InbetweenCell * scell: beforeStar)
    {
        Time t = scell->beforeTime();
        if(res < t)
            res = t;
    }

    return res;
}

Time KeyCell::temporalDragMaxTime() const
{
    //Time res(std::numeric_limits<int>::max()); // Caution: can overflow if compute deltas
    Time res(1000); // TODO


    InbetweenCellSet afterStar = temporalStarAfter();
    for(InbetweenCell * scell: afterStar)
    {
        Time t = scell->afterTime();
        if(t < res)
            res = t;
    }

    return res;
}

void KeyCell::setTime(Time time)
{
    Time minTime = temporalDragMinTime();
    Time maxTime = temporalDragMaxTime();

    if(minTime < time_ && time_ < maxTime)
    {
        time_ = time;
        processGeometryChanged_();
    }
}

void KeyCell::remapPointers(VAC * /*newVAC*/)
{
}

KeyCell::~KeyCell()
{
}

KeyCell::KeyCell(VAC * vac, XmlStreamReader & xml) :
    Cell(vac, xml)
{
    int cellFrame = 0;
    if(xml.attributes().hasAttribute("frame"))
        cellFrame = xml.attributes().value("frame").toInt();

    time_ = Time(cellFrame);
}

KeyCell::KeyCell(VAC * vac, QTextStream & in) :
    Cell(vac, in)
{
    Field field;

    // Time
    in >> field;
    bool timeFieldExists = false;
    if(field.string() == "Time")
        timeFieldExists = true;
    if(timeFieldExists)
    {
        in >> time_ >> field;
    }
    else
    {
        time_ = Time();
    }

    //////// CAUTION: The next field has already been read!!!! ////////


    /*
    // Peek next chars to determine whether there is a "Time" field or not
    bool timeFieldExists = false;
    in.device()->seek(in.pos());
    QByteArray nextChars = in.device()->peek(100);
    QTextStream nextCharsStream(nextChars);
    nextCharsStream >> field;
    if(field.string() == "Time")
        timeFieldExists = true;



    // Set time
    if(timeFieldExists)
    {
        in >> field >> time_;
    }
    else
    {
        time_ = Time();
    }
    */
}

void KeyCell::read2ndPass()
{
}



void KeyCell::save_(QTextStream & out)
{
    // Time
    if(time_ == Time())
    {
        // Default value, no need to save it
    }
    else
    {
        out << Save::newField("Time");
        time_.save(out);
    }
}

void KeyCell::write_(XmlStreamWriter & xml) const
{
    // Time
    if(time_ == Time())
    {
        // Default value, no need to save it
    }
    else
    {
        xml.writeAttribute("frame", QString().setNum(time_.frame()));
    }
}

bool KeyCell::exists(Time t) const
{
    return time() == t;
}

bool KeyCell::isBefore(Time t) const
{
    return time() < t;
}

bool KeyCell::isAfter(Time t) const
{
    return time() > t;
}

bool KeyCell::isAt(Time t) const
{
    return time() == t;
}

KeyCellSet KeyCell::beforeCells() const
{
    return KeyCellSet();
}
KeyCellSet KeyCell::afterCells() const
{
    return KeyCellSet();
}

BoundingBox KeyCell::boundingBox() const
{
    return boundingBox(time());
}

BoundingBox KeyCell::outlineBoundingBox() const
{
    return outlineBoundingBox(time());
}

void KeyCell::drawRaw3D(View3DSettings & /*viewSettings*/)
{
    /*
    glPushMatrix();
    glTranslated(0, 0, floatTime());

    drawRaw(time());

    glPopMatrix();
    */
}

bool KeyCell::checkKey_() const
{
    // todo
    return true;
}

}
