// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2016 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "BackgroundData.h"

BackgroundData::BackgroundData() :
    color(Qt::white),
    imageUrl(""),
    position(0.0, 0.0),
    sizeType(SizeType::Cover),
    size(1280.0, 720.0),
    repeatType(RepeatType::NoRepeat),
    opacity(1.0),
    hold(true)
{
}

bool BackgroundData::operator==(const BackgroundData & other) const
{
    return (color == other.color) &&
           (imageUrl == other.imageUrl) &&
           (position == other.position) &&
           (sizeType == other.sizeType) &&
           (size == other.size) &&
           (repeatType == other.repeatType) &&
           (opacity == other.opacity) &&
           (hold == other.hold);
}

bool BackgroundData::operator!=(const BackgroundData & other) const
{
    return !(*this == other);
}
