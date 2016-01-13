// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2016 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef BACKGROUND_RENDERER_H
#define BACKGROUND_RENDERER_H

#include <QObject>

#include "OpenGL.h"

#include <QMap>

class Background;
class QGLContext;

class BackgroundRenderer: public QObject
{
    Q_OBJECT

public:
    BackgroundRenderer(Background * background,
                       QGLContext * context,
                       QObject * parent = 0);

    void draw(int frame,
              bool showCanvas,
              double canvasWidth, double canvasHeight, double canvasLeft, double canvasTop,
              double xSceneMin, double xSceneMax, double ySceneMin, double ySceneMax);

private slots:
    void clearCache_();

private:
    Background * background_;
    QGLContext * context_;

    GLuint texId_(int frame);
    QMap<int, GLuint> texIds_;
};

#endif // BACKGROUND_RENDERER_H
