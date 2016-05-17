// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef VACRENDERER_H
#define VACRENDERER_H

#include "OpenGL/OpenGLFunctions.h"

#include <QObject>
#include <QMatrix4x4>
#include <QOpenGLVertexArrayObject>

class Vac;
class VacRendererSharedResources;

class VacRenderer: public QObject
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(VacRenderer)

public:
    VacRenderer(VacRendererSharedResources * sharedResources);
    ~VacRenderer();

    VacRendererSharedResources * sharedResources() const;
    Vac * vac() const;

    void initialize(OpenGLFunctions * f);
    void render2D(OpenGLFunctions * f, const QMatrix4x4 & projMatrix, const QMatrix4x4 & viewMatrix);
    void render3D(OpenGLFunctions * f);
    void cleanup(OpenGLFunctions * f);

private:
    // Shared resources
    VacRendererSharedResources * sharedResources_;

    // Context-specific resources
    QOpenGLVertexArrayObject vao_;
};

#endif // VACRENDERER_H
