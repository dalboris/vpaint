// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "View2DRenderer.h"

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
    qDebug() << "View2DRenderer::initialize()"
             << "context =" << QOpenGLContext::currentContext();
}

void View2DRenderer::resize(OpenGLFunctions * f, int w, int h)
{
    qDebug() << "View2DRenderer::resize()"
             << "context =" << QOpenGLContext::currentContext();
}

void View2DRenderer::render(OpenGLFunctions * f)
{
    // XXX TODO call sceneRenderer()->render2D(f);

    qDebug() << "View2DRenderer::render()"
             << "context =" << QOpenGLContext::currentContext();
}

void View2DRenderer::cleanup(OpenGLFunctions * f)
{
    qDebug() << "View2DRenderer::cleanup()"
             << "context =" << QOpenGLContext::currentContext();
}
