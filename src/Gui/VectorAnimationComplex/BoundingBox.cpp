// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "BoundingBox.h"

#include <limits>    // for infinity
#include <algorithm> // for min/max
#include <cmath>     // for abs and isnan

namespace
{
    
const double INF = std::numeric_limits<double>::infinity(); 
const double EPS = 1e-10;

inline bool isnan_(double x)
{
#ifdef WIN32
    return x != x;
#else
    return std::isnan(res);
#endif
}

bool inverted_(double min, double max)
{
    return min > max + EPS;
}

double distance_(double a, double b)
{
    if (a == b) return 0.0; // infinity case, avoid inf-inf = NaN
    else        return std::abs(b-a);
}

double mid_(double min, double max)
{
    double res = 0.5*(min+max);

    // What did the above compute?
    //   * if min and max finite:                          the correct finite   mid-value
    //   * if min finite and max infinite (or vice-versa): the correct infinite mid-value
    //   * if min and max infinite, same sign:             the correct infinite mid-value
    //   * if min and max infinite, different signs:       NaN

    return isnan_(res) ? 0.0 : res;
}

}

namespace VectorAnimationComplex
{
    
BoundingBox::BoundingBox() : 
    xMin_(INF), xMax_(-INF),
    yMin_(INF), yMax_(-INF)
{
}

BoundingBox::BoundingBox(double x, double y) :
    xMin_(x), xMax_(x),
    yMin_(y), yMax_(y)
{
}
    
BoundingBox::BoundingBox(double x1, double x2, double y1, double y2)
{
    if (x1<x2) { xMin_ = x1; xMax_ = x2; }
    else       { xMin_ = x2; xMax_ = x1; }
    
    if (y1<y2) { yMin_ = y1; yMax_ = y2; }
    else       { yMin_ = y2; yMax_ = y1; }
}

bool BoundingBox::isEmpty() const 
{ 
    return inverted_(xMin_, xMax_);
}

bool BoundingBox::isDegenerate() const 
{ 
    return height() <= EPS || width() <= EPS;
}

bool BoundingBox::isInfinite() const 
{ 
    return height() == INF || width() == INF; 
}

bool BoundingBox::isProper() const 
{ 
    return !(isDegenerate() || isInfinite()); 
}

double BoundingBox::xMid() const
{
    return mid_(xMin_, xMax_);
}

double BoundingBox::yMid() const
{
    return mid_(yMin_, yMax_);
}

double BoundingBox::width() const 
{
    return isEmpty() ? 0.0 : distance_(xMin_, xMax_); 
}

double BoundingBox::height() const 
{
    return isEmpty() ? 0.0 : distance_(yMin_, yMax_); 
}

double BoundingBox::area() const 
{ 
    return isDegenerate() ? 0.0 : width() * height(); 
}

BoundingBox BoundingBox::united(const BoundingBox & other) const
{
    BoundingBox res(*this);
    res.unite(other);
    return res;
}
                                       
BoundingBox BoundingBox::intersected(const BoundingBox & other) const
{
    BoundingBox res(*this);
    res.intersect(other);
    return res;
}

void BoundingBox::unite(const BoundingBox & other)
{
    xMin_ = std::min(xMin_, other.xMin_);
    xMax_ = std::max(xMax_, other.xMax_);
    yMin_ = std::min(yMin_, other.yMin_);
    yMax_ = std::max(yMax_, other.yMax_);
}

void BoundingBox::intersect(const BoundingBox & other)
{
    // Compute intersection
    xMin_ = std::max(xMin_, other.xMin_);
    xMax_ = std::min(xMax_, other.xMax_);
    yMin_ = std::max(yMin_, other.yMin_);
    yMax_ = std::min(yMax_, other.yMax_);
    
    // Handle empty special case
    if (inverted_(xMin_, xMax_) || inverted_(yMin_, yMax_))
        *this = BoundingBox();
}

bool BoundingBox::intersects(const BoundingBox & other) const
{
    return !intersected(other).isEmpty();
}

bool BoundingBox::operator==(const BoundingBox & other) const
{
    return distance_(xMin_, other.xMin_) < EPS && 
           distance_(xMax_, other.xMax_) < EPS &&
           distance_(yMin_, other.yMin_) < EPS &&
           distance_(yMax_, other.yMax_) < EPS;
}

bool BoundingBox::operator!=(const BoundingBox & other) const
{
    return !(*this == other);
}

}
