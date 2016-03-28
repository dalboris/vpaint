// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef SCENERENDERER_H
#define SCENERENDERER_H

#include <QObject>

/*
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
*/

class Scene;

class SceneRenderer: public QObject
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(SceneRenderer)

public:
    SceneRenderer(Scene * scene,
                  QObject * parent);

    /* XXX TODO
    void initialize(...);
    void render2D(...);
    void render3D(...);
    void cleanup(...);
    */

private:
    // Scene to render
    Scene * scene_;

    /*

    // OpenGL context
    QOpenGLContext * context_;

    // GPU resources
    QOpenGLVertexArrayObject vao_;
    QOpenGLBuffer vbo_;
    QOpenGLShaderProgram *shaderProgram_;

    // Shader uniform locations
    int projMatrixLoc_;
    int viewMatrixLoc_;
    */
};

#endif // SCENERENDERER_H
