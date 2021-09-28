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

#ifndef BACKGROUND_RENDERER_H
#define BACKGROUND_RENDERER_H

#include <QObject>
#include <QMap>
#include "vpaint_global.h"

class Background;
class QOpenGLContext;
class QOpenGLTexture;

class Q_VPAINT_EXPORT BackgroundRenderer: public QObject
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
