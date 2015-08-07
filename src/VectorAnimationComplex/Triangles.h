// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef TRIANGLES_H
#define TRIANGLES_H

#include "../TimeDef.h"
#include "Eigen.h"
#include <vector>

class View3DSettings;

namespace VectorAnimationComplex
{

inline double cross(const Eigen::Vector2d & p, const Eigen::Vector2d & q)
{
    return p[0]*q[1] - p[1]*q[0];
}

struct Triangle {
    Eigen::Vector2d a, b, c;

    // Check whether a point p is inside the triangle
    bool intersects(const Eigen::Vector2d & p) const;

    // Check whether a rectangle intersects the triangle
    bool intersectsRectangle(double r_minX, double r_maxX, double r_minY, double r_maxY) const;

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

    // Access raw array of values: sizeof(*data) = 9 * sizeof(double) * triangleGeometry.size()
    inline double * data() {return reinterpret_cast<double*>(triangles_.data());}

    // Check whether a point p is included is at least one triangle
    bool intersects(const Eigen::Vector2d & p) const;

    // Check whether a rectangle intersects at least one triangle
    bool intersectsRectangle(double x0, double x1, double y0, double y1) const;

    // Draw
    void draw();
    void draw3D(Time time, View3DSettings & viewSettings);


    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:
    std::vector<Triangle, Eigen::aligned_allocator<Triangle> > triangles_;
};

}

#endif // TRIANGLES_H
