// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "GLUtils.h"

#include <QTransform>
#include <QtDebug>
#include "GLWidget.h"

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

GLuint GLUtils::textureX = 0;
GLuint GLUtils::textureY = 0;
GLuint GLUtils::textureTime = 0;

void GLUtils::drawText(GLuint tex,
                 double x1, double y1, double z1,
                 
                 double x2, double y2, double z2, 
                 double x3, double y3, double z3, 
                 double x4, double y4, double z4)
{
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);
    
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

void GLUtils::genTex(const QString & filename, GLuint & tex)
{
    GLWidget * w = GLWidget::currentGLWidget_;
    if (w)
        tex = w->bindTexture(QPixmap(filename));
        //, GL_TEXTURE_2D, GL_RGBA, 
        //QGLContext::LinearFilteringBindOption | 
        //    QGLContext::InvertedYBindOption);
}

void GLUtils::drawX(double x1, double y1, double z1, 
              double x2, double y2, double z2, 
              double x3, double y3, double z3, 
              double x4, double y4, double z4)
{
    if(!textureX)
        genTex(QString(":/images/letter_x.png"), textureX);
    
    if(!textureX)
        return;
    
    drawText(textureX,
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
        genTex(QString(":/images/letter_y.png"), textureY);
    
    if(!textureY)
        return;
    
    drawText(textureY,
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
        genTex(QString(":/images/string_time.png"), textureTime);
    
    if(!textureTime)
        return;
    
    drawText(textureTime,
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
