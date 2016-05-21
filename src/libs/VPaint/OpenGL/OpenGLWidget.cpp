// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "OpenGL/OpenGLWidget.h"
#include "OpenGL/OpenGLRenderer.h"
#include "OpenGL/OpenGLFunctions.h"

#include <cassert>

OpenGLWidget::OpenGLWidget(QWidget * parent) :
    QOpenGLWidget(parent),
    renderer_()
{
    connect(this, &OpenGLWidget::aboutToCompose, this, &OpenGLWidget::onAboutToCompose);
    connect(this, &OpenGLWidget::frameSwapped, this, &OpenGLWidget::onFrameSwapped);
}

OpenGLWidget::~OpenGLWidget()
{
    cleanup();
}

OpenGLRenderer * OpenGLWidget::renderer() const
{
    return renderer_.get();
}

OpenGLFunctions * OpenGLWidget::functions() const
{
    QOpenGLContext * c = context();
    assert(c);

    OpenGLFunctions * f = c->versionFunctions<OpenGLFunctions>();
    assert(f);

    return f;
}

void OpenGLWidget::cleanup()
{
    makeCurrent();
    OpenGLFunctions * f = functions();
    renderer()->cleanup(f);
    doneCurrent();
}

void OpenGLWidget::initializeGL()
{
    OpenGLFunctions * f = functions();
    f->initializeOpenGLFunctions();
    renderer()->initialize(f);
}

void OpenGLWidget::resizeGL(int w, int h)
{
    OpenGLFunctions * f = functions();
    renderer()->resize(f, w, h);
}

//#include "Core/Debug.h"

void OpenGLWidget::paintGL()
{
    //Debug::log() << "paintGL()";
    OpenGLFunctions * f = functions();
    renderer()->render(f);
}

void OpenGLWidget::onAboutToCompose()
{
    //Debug::log() << "aboutToCompose()";
}

void OpenGLWidget::onFrameSwapped()
{
    //Debug::log() << "frameSwapped()";
}

void OpenGLWidget::setRenderer(OpenGLRenderer * renderer)
{
    renderer_.reset(renderer);
}
