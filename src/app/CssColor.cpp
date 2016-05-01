// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "CssColor.h"
#include <QStringList>
#include <cmath>

CssColor::CssColor(int r, int g, int b, double a) :
    r_(r), g_(g), b_(b), a_(a)
{

}

CssColor::CssColor(const QString & c) :
    r_(0), g_(0), b_(0), a_(1.0)
{
    fromString(c);
}

CssColor::CssColor(const double * c)
{
    setRgbaF(c[0], c[1], c[2], c[3]);
}

int CssColor::r() const { return r_; }
int CssColor::g() const { return g_; }
int CssColor::b() const { return b_; }
double CssColor::a() const { return a_; }

void CssColor::setR(int r) { r_ = r; }
void CssColor::setG(int g) { g_ = g; }
void CssColor::setB(int b) { b_ = b; }
void CssColor::setA(double a) { a_ = a; }

void CssColor::setRgba(int r, int g, int b, double a)
{
    r_ = r;
    g_ = g;
    b_ = b;
    a_ = a;
}

namespace {

int to255(double x) { return std::floor(x*255.0 + 0.5); }
double to1(int x)   { return x * (1.0/255.0); }

}

Color CssColor::toColor() const
{
    return Color(r(), g(), b(), to255(a()));
}

double CssColor::rF() const { return to1(r_); }
double CssColor::gF() const { return to1(g_); }
double CssColor::bF() const { return to1(b_); }
double CssColor::aF() const { return a_; }

void CssColor::setRF(double r) { r_ = to255(r); }
void CssColor::setGF(double g) { g_ = to255(g); }
void CssColor::setBF(double b) { b_ = to255(b); }
void CssColor::setAF(double a) { a_ = a; }

void CssColor::setRgbaF(double r, double g, double b, double a)
{
    r_ = to255(r);
    g_ = to255(g);
    b_ = to255(b);
    a_ = a;
}

void CssColor::fromString(const QString & c)
{
    // Remove all whitespaces, e.g.:
    //   "  rgba ( 127,0  , 255, 1.0) " -> "rgba(127,0,255,1.0)"
    QString d;
    for(int i=0; i<c.length(); ++i)
        if(!c.at(i).isSpace())
            d += c.at(i);

    // Get relevant data: "rgba(127,0,255,1.0)" -> "127,0,255,1.0"
    d.remove(0,5).chop(1);

    // Split: "127,0,255,1.0" -> [ "127" ; "0" ; "255" ; "1.0" ]
    QStringList l = d.split(',');

    // Write data to members
    setRgba(l[0].toInt(), l[1].toInt(), l[2].toInt(), l[3].toDouble());
}

QString CssColor::toString() const
{
    return
            QString("rgba(") +
            QString().setNum(r_) + QString(",") +
            QString().setNum(g_) + QString(",") +
            QString().setNum(b_) + QString(",") +
            QString().setNum(a_,'g',2) + QString(")");
}
