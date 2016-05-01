// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "View2DRenderer.h"
#include "Scene/SceneRenderer.h"

#include <QOpenGLContext>
#include <QtDebug>

View2DRenderer::View2DRenderer(
        SceneRenderer * sceneRenderer,
        QObject * parent) :
    OpenGLRenderer(parent),
    sceneRenderer_(sceneRenderer)
{
}

SceneRenderer * View2DRenderer::sceneRenderer()
{
    return sceneRenderer_;
}

void View2DRenderer::initialize(OpenGLFunctions * f)
{
    sceneRenderer()->initialize(f);
}

void View2DRenderer::resize(OpenGLFunctions * /*f*/, int w, int h)
{
    // Set projection matrix
    const float left   = 0.0f;
    const float right  = w;
    const float bottom = h;
    const float top    = 0.0f;
    const float near   = -1.0f;
    const float far    = 1.0f;
    QMatrix4x4 projMat;
    projMat.ortho(left, right, bottom, top, near, far);
    setProjectionMatrix(projMat);

    // Set view matrix
    QMatrix4x4 viewMat;     // = identity
    setViewMatrix(viewMat);
}

void View2DRenderer::render(OpenGLFunctions * f)
{
    sceneRenderer()->render2D(f, projectionMatrix(), viewMatrix());
}

void View2DRenderer::cleanup(OpenGLFunctions * f)
{
    sceneRenderer()->cleanup(f);
}
