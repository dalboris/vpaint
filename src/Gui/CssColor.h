// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef CSSCOLOR_H
#define CSSCOLOR_H

#include "Color.h"

#include <QString>

class CssColor
{
public:
    // Constructors
    CssColor(int r=0, int g=0, int b=0, double a=1.0);
    CssColor(const QString & c);

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
