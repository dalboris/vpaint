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

#ifndef COLOR_H
#define COLOR_H

// Note: since it's a typedef, you can't forward-declare it as 'class Color;',
//       so you must '#include "Color.h"'

#include <QColor>

typedef QColor Color;

Color lerp(const Color & c0, const Color & c1, double u); // u=0 => c0 ; u=1 => c1

#endif // COLOR_H
