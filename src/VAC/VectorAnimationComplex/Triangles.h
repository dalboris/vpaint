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

#ifndef TRIANGLES_H
#define TRIANGLES_H

#include "../TimeDef.h"
#include "Eigen.h"
#include "BoundingBox.h"
#include <vector>
#include "vpaint_global.h"

class View3DSettings;

namespace VectorAnimationComplex
{

class BoundingBox;

inline double cross(const Eigen::Vector2d & p, const Eigen::Vector2d & q)
{
    return p[0]*q[1] - p[1]*q[0];
}

struct Q_VPAINT_EXPORT Triangle {
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

class Q_VPAINT_EXPORT Triangles
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
    inline int size() const {return (int)triangles_.size();}
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
