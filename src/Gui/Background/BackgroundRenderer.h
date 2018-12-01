// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef BACKGROUND_RENDERER_H
#define BACKGROUND_RENDERER_H

#include <QObject>

#include "../OpenGL.h"

#include <QMap>

class Background;
class QOpenGLContext;
class QOpenGLTexture;

class BackgroundRenderer: public QObject
{
    Q_OBJECT

public:
    BackgroundRenderer(Background * background,
                       QObject * parent = 0);

    // Destroys allocated GPU resources. This requires a current valid OpenGL
    // context.
    //
    void cleanup();

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

signals:
    void backgroundDestroyed(Background * background);

private slots:
    void setDirty_();
    void clearCache_();
    void onBackgroundDestroyed_();

private:
    Background * background_;

    bool isCacheDirty_;

    QOpenGLTexture * texture_(int frame);
    QMap<int, QOpenGLTexture *> textures_;
};

#endif // BACKGROUND_RENDERER_H
