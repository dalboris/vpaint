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

#include "GLUtils.h"

#include <QCoreApplication>
#include <QTransform>
#include <QtDebug>
#include <QOpenGLTexture>
#include <QSurfaceFormat>

#include "OpenGL.h"

/*
void GLUtils::UnitCircleZ()
{
}

void GLUtils::UnitSphere()
{
    GLUquadric* quad = gluNewQuadric();
    int slices = 50;
    int stacks = 20;
    double radius = 1;
    gluSphere(quad, radius, slices, stacks);
}

void GLUtils::Sphere(double radius)
{
    GLUquadric* quad = gluNewQuadric();
    int slices = 50;
    int stacks = 20;
    gluSphere(quad, radius, slices, stacks);
}
*/

QOpenGLTexture * GLUtils::textureX = nullptr;
QOpenGLTexture * GLUtils::textureY = nullptr;
QOpenGLTexture * GLUtils::textureTime = nullptr;

void GLUtils::init()
{
    // Performance seems to be significantly impacted by format.setSamples().
    // For now I keep setSamples(1). May change to 4 or 16 after investigation.

    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

    QSurfaceFormat format;
    format.setVersion(VPAINT_OPENGL_VERSION_MAJOR, VPAINT_OPENGL_VERSION_MINOR);
    format.setOption(QSurfaceFormat::DeprecatedFunctions);
    format.setDepthBufferSize(24);
    format.setRedBufferSize(8);
    format.setGreenBufferSize(8);
    format.setBlueBufferSize(8);
    format.setAlphaBufferSize(0);
    format.setStencilBufferSize(8);
    format.setSamples(1);
    format.setSwapBehavior(QSurfaceFormat::DefaultSwapBehavior);
    format.setSwapInterval(0);
    format.setColorSpace(QSurfaceFormat::DefaultColorSpace);
    format.setProfile(QSurfaceFormat::CompatibilityProfile);
    QSurfaceFormat::setDefaultFormat(format);
}

void GLUtils::drawTex(QOpenGLTexture * tex,
                 double x1, double y1, double z1,
                 
                 double x2, double y2, double z2, 
                 double x3, double y3, double z3, 
                 double x4, double y4, double z4)
{
    if (!tex) {
        return;
    }

    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    tex->bind();
    
    glBegin(GL_QUADS);
    {
        glTexCoord2f(0, 0);
        glVertex3f(x1,y1,z1);
        
        glTexCoord2f(1, 0);
        glVertex3f(x2,y2,z2);
        
        glTexCoord2f(1, 1);
        glVertex3f(x3,y3,z3);
        
        glTexCoord2f(0, 1);
        glVertex3f(x4,y4,z4);
    }
    glEnd();
    
    glDisable(GL_TEXTURE_2D);
}

QOpenGLTexture * GLUtils::genTex(const QString & filename)
{
    return new QOpenGLTexture(QImage(filename).mirrored());
}

void GLUtils::drawX(double x1, double y1, double z1, 
              double x2, double y2, double z2, 
              double x3, double y3, double z3, 
              double x4, double y4, double z4)
{
    if(!textureX)
        textureX = genTex(QString(":/images/letter_x.png"));
    
    drawTex(textureX,
           x1, y1, z1,
           x2, y2, z2, 
           x3, y3, z3, 
           x4, y4, z4);
}

void GLUtils::drawY(double x1, double y1, double z1, 
              double x2, double y2, double z2, 
              double x3, double y3, double z3, 
              double x4, double y4, double z4)
{
    if(!textureY)
        textureY = genTex(QString(":/images/letter_y.png"));
    
    drawTex(textureY,
           x1, y1, z1,
           x2, y2, z2, 
           x3, y3, z3, 
           x4, y4, z4);
}

void GLUtils::drawTime(double x1, double y1, double z1, 
              double x2, double y2, double z2, 
              double x3, double y3, double z3, 
              double x4, double y4, double z4)
{
    if(!textureTime)
        textureTime = genTex(QString(":/images/string_time.png"));
    
    drawTex(textureTime,
           x1, y1, z1,
           x2, y2, z2, 
           x3, y3, z3, 
           x4, y4, z4);
}

void GLUtils::multMatrix_RectInsideRect(const QRectF & rect1,
                            const QRectF & rect2)
{
    
    QPolygonF p1from(rect1);
    p1from.remove(4);
    double ratio1 = rect1.width() / rect1.height();
    QRectF destRect1 = rect2;
    if(ratio1<1)
    {
        destRect1.setWidth(ratio1*destRect1.height());
        destRect1.moveLeft(destRect1.left()+0.5*(destRect1.height()-destRect1.width()));
    }
    else
    {
        destRect1.setHeight(destRect1.width()/ratio1);
        destRect1.moveTop(destRect1.top()+0.5*(destRect1.width()-destRect1.height()));
    }
    QPolygonF p1to(destRect1);
    p1to.remove(4);
    GLUtils::multMatrix_QuadToQuad(p1from,p1to);
}

void GLUtils::multMatrix_RectToRect(const QRectF & rect1,
                        const QRectF & rect2)
{
    QPolygonF p1(rect1);
    p1.remove(4);
    QPolygonF p2(rect2);
    p2.remove(4);
    GLUtils::multMatrix_QuadToQuad(p1,p2);
}

void GLUtils::multMatrix_QuadToQuad(const QPolygonF & quad1,
                        const QPolygonF & quad2)
{
    // the transform to apply
    QTransform t;
    if(!QTransform::quadToQuad(quad1, quad2, t))
        qDebug() << "ERROR in GLUtils::multMatrix_QuadToQuad()";
    

    // the OpenGL matrix
    GLdouble mat[4][4];
    
    mat[0][0] = t.m11();
    mat[0][1] = t.m12();
    mat[0][2] = 0;
    mat[0][3] = t.m13();
    
    mat[1][0] = t.m21();
    mat[1][1] = t.m22();
    mat[1][2] = 0;
    mat[1][3] = t.m23();

    mat[2][0] = 0;
    mat[2][1] = 0;
    mat[2][2] = 1;
    mat[2][3] = 0;

    mat[3][0] = t.m31();
    mat[3][1] = t.m32();
    mat[3][2] = 0;
    mat[3][3] = t.m33();
    
    // do the mutiplication
    glMultMatrixd(reinterpret_cast<GLdouble*>(mat));
}

void GLUtils::drawArrow(const Eigen::Vector2d & p, const Eigen::Vector2d & u)
{
    Eigen::Vector2d v(-u[1],u[0]);
    Eigen::Vector2d A = p - 5*u + 5*v;
    Eigen::Vector2d C = p - 5*u - 5*v;
    glColor3d(1,0,0);
    glLineWidth(3);
    glBegin(GL_LINE_STRIP);
    {
        glVertex2d(A[0],A[1]);
        glVertex2d(p[0],p[1]);
        glVertex2d(C[0],C[1]);
    }
    glEnd();
}
