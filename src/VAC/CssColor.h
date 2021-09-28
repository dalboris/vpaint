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

#ifndef CSSCOLOR_H
#define CSSCOLOR_H

#include "Color.h"

#include <QString>
#include "vpaint_global.h"

class Q_VPAINT_EXPORT CssColor
{
public:
    // Constructors
    CssColor(int r=0, int g=0, int b=0, double a=1.0); // expects RGB in [0,255] and A in [0,1]
    CssColor(const QString & c);                       // expects string of the form "rgba(r,b,b,a)", same ranges as above
    CssColor(const double * c);                        // expects an array of size 4 with RGBA values all in [0,1]

    // Get
    int r() const;
    int g() const;
    int b() const;
    double a() const;

    double rF() const;
    double gF() const;
    double bF() const;
    double aF() const;

    Color toColor() const;

    // Set
    void setRgba(int r, int g, int b, double a);
    void setR(int r);
    void setG(int g);
    void setB(int b);
    void setA(double a);

    void setRgbaF(double r, double g, double b, double a);
    void setRF(double r);
    void setGF(double g);
    void setBF(double b);
    void setAF(double a);

    // String input/output as "rgba(r,g,b,a)"
    QString toString() const;
    void fromString(const QString & c);

private:
    int r_, g_, b_; // [0  , 255]
    double a_;    // [0.0, 1.0]
};

#endif // CSSCOLOR_H
