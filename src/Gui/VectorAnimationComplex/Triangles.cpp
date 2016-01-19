// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "Triangles.h"
#include "../OpenGL.h"

#include "../View3DSettings.h"

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
inline void threeWayMinMax(double a, double b, double c, double & min, double & max)
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
inline bool projectionIntersects(double ux, double uy,
                                 double r_minX, double r_maxX, double r_minY, double r_maxY,
                                 double tx, double ty)
{
    // Convenient aliases for the coordinates of the rectangle
    double & ax = r_minX;
    double & bx = r_minX;
    double & cx = r_maxX;
    double & dx = r_maxX;
    double & ay = r_minY;
    double & by = r_maxY;
    double & cy = r_maxY;
    double & dy = r_minY;

    // Compute non-normalized projections along u axis
    double a = ux*ax + uy*ay;
    double b = ux*bx + uy*by;
    double c = ux*cx + uy*cy;
    double d = ux*dx + uy*dy;
    double t = ux*tx + uy*ty;

    // Sorting
    double minT, maxT;
    if(t<0)    { minT = t; maxT = 0; }
    else    { minT = 0; maxT = t; }
    double minR = std::min( a, std::min( b, std::min(c,d) ) );
    double maxR = std::max( a, std::max( b, std::max(c,d) ) );

    // Testing intersection
    if(minR > maxT) return false;
    if(maxR < minT) return false;

    // In any other case
    return true;
}

}

bool Triangle::intersectsRectangle(double r_minX, double r_maxX, double r_minY, double r_maxY) const
{
    // Using the Separation Axis Theorem

    // Test against rectangle axes
    double t_minX, t_maxX, t_minY, t_maxY;
    threeWayMinMax(a[0],b[0],c[0],t_minX,t_maxX);
    threeWayMinMax(a[1],b[1],c[1],t_minY,t_maxY);
    if(t_minX > r_maxX) return false;
    if(t_maxX < r_minX) return false;
    if(t_minY > r_maxY) return false;
    if(t_maxY < r_minY) return false;

    // Test against triangle axes
    if(!projectionIntersects(a[1]-b[1], b[0]-a[0],
                             r_minX-a[0], r_maxX-a[0], r_minY-a[1], r_maxY-a[1],
                             c[0]-a[0], c[1]-a[1])) return false;
    if(!projectionIntersects(b[1]-c[1], c[0]-b[0],
                             r_minX-b[0], r_maxX-b[0], r_minY-b[1], r_maxY-b[1],
                             a[0]-b[0], a[1]-b[1])) return false;
    if(!projectionIntersects(c[1]-a[1], a[0]-c[0],
                             r_minX-c[0], r_maxX-c[0], r_minY-c[1], r_maxY-c[1],
                             b[0]-c[0], b[1]-c[1])) return false;

    // In all other cases
    return true;
}

bool Triangles::intersects(const Eigen::Vector2d & p) const
{
    for(const Triangle & t : triangles_)
    {
        bool b = t.intersects(p);
        if(b) return true;
    }

    return false;
}

bool Triangles::intersectsRectangle(double x0, double x1, double y0, double y1) const
{
    for(const Triangle & t : triangles_)
    {
        bool b = t.intersectsRectangle(x0,x1,y0,y1);
        if(b) return true;
    }

    return false;
}

void Triangles::draw()
{
    // Drawing
    glBegin(GL_TRIANGLES);
    for(unsigned int i=0; i<triangles_.size(); ++i)
    {
        glVertex2d(triangles_[i].a[0],triangles_[i].a[1]);
        glVertex2d(triangles_[i].b[0],triangles_[i].b[1]);
        glVertex2d(triangles_[i].c[0],triangles_[i].c[1]);
    }
    glEnd();
}

void Triangles::draw3D(Time time, View3DSettings & viewSettings)
{
    double z = viewSettings.zFromT(time);

    // Drawing
    glBegin(GL_TRIANGLES);
    for(unsigned int i=0; i<triangles_.size(); ++i)
    {
        glVertex3d(viewSettings.xFromX2D(triangles_[i].a[0]), viewSettings.yFromY2D(triangles_[i].a[1]), z);
        glVertex3d(viewSettings.xFromX2D(triangles_[i].b[0]), viewSettings.yFromY2D(triangles_[i].b[1]), z);
        glVertex3d(viewSettings.xFromX2D(triangles_[i].c[0]), viewSettings.yFromY2D(triangles_[i].c[1]), z);
    }
    glEnd();
}

}
