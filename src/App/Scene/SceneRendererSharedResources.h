// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef SCENERENDERERSHAREDRESOURCES_H
#define SCENERENDERERSHAREDRESOURCES_H

#include "OpenGL/OpenGLFunctions.h"
#include "Core/Memory.h"

#include <QObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

class Scene;

class SceneRendererSharedResources: public QObject
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(SceneRendererSharedResources)

public:
    SceneRendererSharedResources(
            Scene * scene,
            QObject * parent);

    Scene * scene() const { return scene_; }

    void initialize(OpenGLFunctions * f);
    void update(OpenGLFunctions * f);
    void cleanup(OpenGLFunctions * f);

public slots:
    void setDirty();

private:
    friend class SceneRenderer;

    // Scene to render
    Scene * scene_;

    // GPU resources
    UniquePtr<QOpenGLShaderProgram> shaderProgram_;
    QOpenGLBuffer vbo_;

    // Shader locations
    int vertexLoc_;
    int projMatrixLoc_;
    int viewMatrixLoc_;

    // Reference counting to initialize and cleanup only once
    int refCount_ = 0;

    // Dirty bit
    bool isDirty_ = true;
};

#endif // SCENERENDERERSHAREDRESOURCES_H
