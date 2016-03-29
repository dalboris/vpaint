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

#include "Core/Memory.h"
#include "OpenGL/OpenGLFunctions.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QMatrix4x4>

class Scene;

class SceneRenderer: public QObject
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(SceneRenderer)

public:
    SceneRenderer(Scene * scene,
                  QObject * parent);

    ~SceneRenderer();

    Scene * scene() const;

    void initialize(OpenGLFunctions * f);
    void render2D(OpenGLFunctions * f, const QMatrix4x4 & projMatrix, const QMatrix4x4 & viewMatrix);
    void render3D(OpenGLFunctions * f);
    void cleanup(OpenGLFunctions * f);

private:
    // Scene to render
    Scene * scene_;

    // GPU resources
    QOpenGLBuffer vbo_;
    UniquePtr<QOpenGLVertexArrayObject> vao_;
    UniquePtr<QOpenGLShaderProgram> shaderProgram_;

    // Shader locations
    int vertexLoc_;
    int projMatrixLoc_;
    int viewMatrixLoc_;
};

#endif // SCENERENDERER_H
