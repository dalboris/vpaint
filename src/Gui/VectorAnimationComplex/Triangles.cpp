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

#include "Triangles.h"

#include "../OpenGL.h"
#include "../View3DSettings.h"
#include <limits>

namespace VectorAnimationComplex
{

Triangles::Triangles() :
    triangles_()
{
}

bool Triangle::intersects(const Eigen::Vector2d & p) const
{
    double a1 = cross(b-a,p-a);
    double a2 = cross(c-b,p-b);
    double a3 = cross(a-c,p-c);

    return (a1 >= 0 && a2 >=0 && a3 >= 0) || (a1 <= 0 && a2 <=0 && a3 <= 0);
}

namespace
{

void threeWayMinMax(double a, double b, double c, double & min, double & max)
{
    if(a<b)
    {
        if(a<c)
        {
            min = a;
            max = (b<c) ? c : b;
        }
        else
        {
            min = c;
            max = b;
        }
    }
    else
    {
        if(b<c)
        {
            min = b;
            max = (a<c) ? c : a;
        }
        else
        {
            min = c;
            max = a;
        }
    }
}

bool projectionIntersects(double ux, double uy,
                          double r_xMin, double r_xMax, double r_yMin, double r_yMax,
                          double tx, double ty)
{
    // Convenient aliases for the coordinates of the rectangle
    const double & ax = r_xMin;
    const double & bx = r_xMin;
    const double & cx = r_xMax;
    const double & dx = r_xMax;
    const double & ay = r_yMin;
    const double & by = r_yMax;
    const double & cy = r_yMax;
    const double & dy = r_yMin;

    // Compute non-normalized projections along u axis
    const double a = ux*ax + uy*ay;
    const double b = ux*bx + uy*by;
    const double c = ux*cx + uy*cy;
    const double d = ux*dx + uy*dy;
    const double t = ux*tx + uy*ty;

    // Sorting
    double minT, maxT;
    if (t<0) { minT = t; maxT = 0; }
    else     { minT = 0; maxT = t; }
    const double minR = std::min( a, std::min( b, std::min(c,d) ) );
    const double maxR = std::max( a, std::max( b, std::max(c,d) ) );

    // Testing intersection
    if(minR > maxT) return false;
    if(maxR < minT) return false;

    // In any other case
    return true;
}

}

bool Triangle::intersects(const BoundingBox & bb) const
{
    // Triangle-Rectangle intersection test.
    // It is implemented using the Separation Axis Theorem (SAT).

    // Get aliases for bounding box boundaries
    const double & r_xMin = bb.xMin();
    const double & r_xMax = bb.xMax();
    const double & r_yMin = bb.yMin();
    const double & r_yMax = bb.yMax();

    // Test against rectangle axes
    double t_xMin, t_xMax, t_yMin, t_yMax;
    threeWayMinMax(a[0],b[0],c[0],t_xMin,t_xMax);
    threeWayMinMax(a[1],b[1],c[1],t_yMin,t_yMax);
    if(t_xMin > r_xMax) return false;
    if(t_xMax < r_xMin) return false;
    if(t_yMin > r_yMax) return false;
    if(t_yMax < r_yMin) return false;

    // Test against triangle axes
    if(!projectionIntersects(a[1]-b[1], b[0]-a[0],
                             r_xMin-a[0], r_xMax-a[0], r_yMin-a[1], r_yMax-a[1],
                             c[0]-a[0], c[1]-a[1])) return false;
    if(!projectionIntersects(b[1]-c[1], c[0]-b[0],
                             r_xMin-b[0], r_xMax-b[0], r_yMin-b[1], r_yMax-b[1],
                             a[0]-b[0], a[1]-b[1])) return false;
    if(!projectionIntersects(c[1]-a[1], a[0]-c[0],
                             r_xMin-c[0], r_xMax-c[0], r_yMin-c[1], r_yMax-c[1],
                             b[0]-c[0], b[1]-c[1])) return false;

    // In all other cases
    return true;
}

bool Triangles::intersects(const Eigen::Vector2d & p) const
{
    for (const Triangle & t : triangles_)
        if (t.intersects(p))
            return true;

    return false;
}

bool Triangles::intersects(const BoundingBox & bb) const
{
    for (const Triangle & t : triangles_)
        if (t.intersects(bb))
            return true;

    return false;
}

BoundingBox Triangle::boundingBox() const
{
    double x1, x2, y1, y2;
    threeWayMinMax(a[0], b[0], c[0], x1, x2);
    threeWayMinMax(a[1], b[1], c[1], y1, y2);
    return BoundingBox(x1, x2, y1, y2);
}

BoundingBox Triangles::boundingBox() const
{
    BoundingBox bb;
    for (const Triangle & t : triangles_)
        bb.unite(t.boundingBox());
    return bb;
}

void Triangles::draw() const
{
    glBegin(GL_TRIANGLES);
    for (unsigned int i=0; i<triangles_.size(); ++i)
    {
        glVertex2d(triangles_[i].a[0], triangles_[i].a[1]);
        glVertex2d(triangles_[i].b[0], triangles_[i].b[1]);
        glVertex2d(triangles_[i].c[0], triangles_[i].c[1]);
    }
    glEnd();
}

void Triangles::draw3D(Time t, View3DSettings & viewSettings) const
{
    const double z = viewSettings.zFromT(t);

    glBegin (GL_TRIANGLES);
    for (unsigned int i=0; i<triangles_.size(); ++i)
    {
        glVertex3d(viewSettings.xFromX2D(triangles_[i].a[0]), viewSettings.yFromY2D(triangles_[i].a[1]), z);
        glVertex3d(viewSettings.xFromX2D(triangles_[i].b[0]), viewSettings.yFromY2D(triangles_[i].b[1]), z);
        glVertex3d(viewSettings.xFromX2D(triangles_[i].c[0]), viewSettings.yFromY2D(triangles_[i].c[1]), z);
    }
    glEnd();
}

}
