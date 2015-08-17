// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef COLOR_H
#define COLOR_H

#include <QColor>

typedef QColor Color;

Color lerp(const Color & c0, const Color & c1, double u); // u=0 => c0 ; u=1 => c1

#endif // COLOR_H
