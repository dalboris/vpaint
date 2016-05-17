// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef VACRENDERERSHAREDRESOURCES_H
#define VACRENDERERSHAREDRESOURCES_H

#include "OpenGL/OpenGLFunctions.h"

#include <QObject>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>

class Vac;

class VacRendererSharedResources: public QObject
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(VacRendererSharedResources)

public:
    VacRendererSharedResources(Vac * vac);
    ~VacRendererSharedResources();

    Vac * vac() const;

    void initialize(OpenGLFunctions * f);
    void update(OpenGLFunctions * f);
    void cleanup(OpenGLFunctions * f);

public slots:
    void setDirty();

private:
    friend class VacRenderer;

    // Vac to render
    Vac * vac_;

    // GPU resources
    QOpenGLShaderProgram shaderProgram_;
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

#endif // VACRENDERERSHAREDRESOURCES_H
