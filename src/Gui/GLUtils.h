// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef GLUTILS_H
#define GLUTILS_H

#include "OpenGL.h"
#include <QPolygonF>
#include <QRectF>

#include <Eigen/Core>

class GLUtils
{
public:
    //static void UnitCircleZ();
    //static void UnitSphere();
    //static void Sphere(double radius);
    static void multMatrix_RectInsideRect(const QRectF & rect1,
                              const QRectF & rect2);
    static void multMatrix_RectToRect(const QRectF & rect1,
                            const QRectF & rect2);
    static void multMatrix_QuadToQuad(const QPolygonF & quad1,
                            const QPolygonF & quad2);

    static void drawX(double x1, double y1, double z1, 
                double x2, double y2, double z2, 
                double x3, double y3, double z3, 
                double x4, double y4, double z4);
    static void drawY(double x1, double y1, double z1, 
                double x2, double y2, double z2, 
                double x3, double y3, double z3, 
                double x4, double y4, double z4);
    static void drawTime(double x1, double y1, double z1, 
                   double x2, double y2, double z2, 
                   double x3, double y3, double z3, 
                   double x4, double y4, double z4);

    static void drawArrow(const Eigen::Vector2d & p, const Eigen::Vector2d & u);
    
private:
    // drawing text
    static void genTex(const QString & filename, GLuint & tex);
    static void drawText(GLuint tex,
                   double x1, double y1, double z1, 
                   double x2, double y2, double z2, 
                   double x3, double y3, double z3, 
                   double x4, double y4, double z4);
    static GLuint textureX;
    static GLuint textureY;
    static GLuint textureTime;
};

#endif
