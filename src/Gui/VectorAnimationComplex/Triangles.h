// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef TRIANGLES_H
#define TRIANGLES_H

#include "../TimeDef.h"
#include "Eigen.h"
#include "BoundingBox.h"
#include <vector>

class View3DSettings;

namespace VectorAnimationComplex
{

class BoundingBox;

inline double cross(const Eigen::Vector2d & p, const Eigen::Vector2d & q)
{
    return p[0]*q[1] - p[1]*q[0];
}

struct Triangle {
    Triangle() {}

    Triangle(const Eigen::Vector2d & a_,
             const Eigen::Vector2d & b_,
             const Eigen::Vector2d & c_) :
        a(a_), b(b_), c(c_)
    {
    }

    Eigen::Vector2d a, b, c;

    // Check whether a point p is inside the triangle
    bool intersects(const Eigen::Vector2d & p) const;

    // Check whether a rectangle intersects the triangle
    bool intersects(const BoundingBox & bb) const;

    // Compute bounding box
    BoundingBox boundingBox() const;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

class Triangles
{
public:
    // Build an empty vector of triangles
    Triangles();

    // Clear
    inline void clear() {triangles_.clear();}

    // Append a triangle
    inline Triangles & operator<< (const Triangle & t)
    {
        triangles_.push_back(t);
        return *this;
    }
    inline void append(double ax, double ay,
                       double bx, double by,
                       double cx, double cy)
    {
        Triangle t;
        t.a[0] = ax;
        t.a[1] = ay;
        t.b[0] = bx;
        t.b[1] = by;
        t.c[0] = cx;
        t.c[1] = cy;
        triangles_.push_back(t);
    }

    // Access and modify content
    inline int size() const {return triangles_.size();}
    inline Triangle & operator[] (int i) {return triangles_[i];}

    // Access raw data
    inline double * data() {return reinterpret_cast<double*>(triangles_.data());}

    // Check whether a point p is included is at least one triangle
    bool intersects(const Eigen::Vector2d & p) const;

    // Check whether a rectangle intersects at least one triangle
    bool intersects(const BoundingBox & bb) const;

    // Compute bounding box
    BoundingBox boundingBox() const;

    // Draw
    void draw() const;
    void draw3D(Time t, View3DSettings & viewSettings) const;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:
    std::vector<Triangle, Eigen::aligned_allocator<Triangle>> triangles_;
};

}

#endif // TRIANGLES_H
