// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "Color.h"

Color lerp(const Color & c0, const Color & c1, double u)
{
    // Get c0 HSL
    double h0 = c0.hueF();
    double s0 = c0.saturationF();
    double l0 = c0.lightnessF();
    double a0 = c0.alphaF();

    // Get c1 HSL
    double h1 = c1.hueF();
    double s1 = c1.saturationF();
    double l1 = c1.lightnessF();
    double a1 = c1.alphaF();

    // Compute lerp
    Color res;
    res.setHslF( h0+u*(h1-h0) , s0+u*(s1-s0) , l0+u*(l1-l0) , a0+u*(a1-a0) );

    // Return
    return res;
}
