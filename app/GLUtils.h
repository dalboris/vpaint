// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef GLUTILS_H
#define GLUTILS_H

#include "OpenGL.h"
#include <QPolygonF>
#include <QRectF>

#include "Eigen/Core"

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
