// Copyright (C) 2012-2019 The VPaint Developers
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

#ifndef GLUTILS_H
#define GLUTILS_H

#include <QPolygonF>
#include <QRectF>

#include <Eigen/Core>

class QOpenGLTexture;

class GLUtils
{
public:
    /// This function must be called before creating the first
    /// OpenGLWidget. It sets the appropriate Qt OpenGLFormat and
    /// sets the Qt::AA_UseDesktopOpenGL attribute.
    static void init();

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
    static QOpenGLTexture * genTex(const QString & filename);
    static void drawTex(QOpenGLTexture * tex,
                   double x1, double y1, double z1, 
                   double x2, double y2, double z2, 
                   double x3, double y3, double z3, 
                   double x4, double y4, double z4);
    static QOpenGLTexture * textureX;
    static QOpenGLTexture * textureY;
    static QOpenGLTexture * textureTime;
};

#endif
