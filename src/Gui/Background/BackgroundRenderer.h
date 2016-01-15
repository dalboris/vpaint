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

    // Draw the background for specified frame.
    //
    // If showCanvas = true, then draw an area covering the canvas only, and
    // the variables xSceneMin, xSceneMax, ySceneMin, and ySceneMax are unused.
    //
    // If showCanvas = false, then draw an area covering the whole screen. Though,
    // it still the canvas dimensions and positions, to know the position and size
    // of he background image.
    //
    // 3D view should either use showCanvas = true, or not draw the background
    // at all, since showCanvas = false would paint the whole window with the
    // background color, which doesn't make sense.
    //
    // XXX We should probably pass a pointer to a canvas object in the
    // constructor, so we don't have to pass that many parameters. (but the
    // 'Canvas' class is not even implemented yet)
    //
    void draw(int frame,bool showCanvas,

              double canvasLeft, double canvasTop,
              double canvasWidth, double canvasHeight,

              double xSceneMin, double xSceneMax,
              double ySceneMin, double ySceneMax);

private slots:
    void clearCache_();

private:
    Background * background_;
    QGLContext * context_;

    GLuint texId_(int frame);
    QMap<int, GLuint> texIds_;
};

#endif // BACKGROUND_RENDERER_H
