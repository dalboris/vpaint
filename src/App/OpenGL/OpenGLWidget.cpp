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

#include <QtDebug>

OpenGLWidget::OpenGLWidget(QWidget * parent) :
    QOpenGLWidget(parent),
    renderer_(nullptr)
  /*,
    shaderProgram_(0)
      */
{
    /*
    data_.push_back(glm::vec2(50, 50));
    data_.push_back(glm::vec2(50, 100));
    data_.push_back(glm::vec2(100, 50));
    */
}

OpenGLWidget::~OpenGLWidget()
{
    cleanup();
}

void OpenGLWidget::setRenderer(OpenGLRenderer * renderer)
{
    assert(renderer);
    renderer_ = renderer;
}

OpenGLRenderer * OpenGLWidget::renderer() const
{
    assert(renderer_);
    return renderer_;
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

    // XXX todo
    // -> this is state that depends on each view!
    // So each View2D must have a "View2DRenderer"
    // with its own state, and implementation of
    // resize(). This renderer should forward render()
    // to the shared SceneRenderer()->render2D()
}

void OpenGLWidget::paintGL()
{
    qint64 elapsed = elapsedTimer_.elapsed();
    elapsedTimer_.restart();
    qDebug() << "OpenGLWidget::paintGL() elapsed =" << elapsed;

    OpenGLFunctions * f = functions();
    renderer()->render(f);
}










/*
const QMatrix4x4 & OpenGLWidget::projectionMatrix() const
{
    return projMatrix_;
}

const QMatrix4x4 & OpenGLWidget::projectionMatrixInverse() const
{
    if (projMatrixInv_.isDirty())
    {
        projMatrixInv_.setValue(projMatrix_.inverted());
    }
    return projMatrixInv_.value();
}

void OpenGLWidget::setProjectionMatrix(const QMatrix4x4 & projectionMatrix)
{
    projMatrix_ = projectionMatrix;
    projMatrixInv_.setDirty();
}

const QMatrix4x4 & OpenGLWidget::viewMatrix() const
{
    return viewMatrix_;
}

const QMatrix4x4 & OpenGLWidget::viewMatrixInverse() const
{
    if (viewMatrixInv_.isDirty())
    {
        viewMatrixInv_.setValue(viewMatrix_.inverted());
    }
    return viewMatrixInv_.value();
}

void OpenGLWidget::setViewMatrix(const QMatrix4x4 & viewMatrix)
{
    viewMatrix_ = viewMatrix;
    viewMatrixInv_.setDirty();
}
*/

/*
void OpenGLWidget::cleanup()
{
    makeCurrent();
    renderer_->cleanup();
    vbo_.destroy();
    vao_.reset();
    shaderProgram_.reset();
    doneCurrent();
}

void OpenGLWidget::initializeGL()
{
    QOpenGLFunctions * glFuncs = QOpenGLContext::currentContext()->functions();

    // Ensure proper cleanup
    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &OpenGLWidget::cleanup);

    // Initialize OpenGL functions
    initializeOpenGLFunctions();

    // Set clear color
    glClearColor(1, 1, 1, 1);

    // Allocate shader program
    shaderProgram_ = new QOpenGLShaderProgram;
    shaderProgram_->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/OpenGL/Shaders/Helloworld.v.glsl");
    shaderProgram_->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/OpenGL/Shaders/Helloworld.f.glsl");
    shaderProgram_->bindAttributeLocation("vertex", 0);
    shaderProgram_->link();

    // Bind shader program
    shaderProgram_->bind();

    // Get shader uniform locations
    projMatrixLoc_ = shaderProgram_->uniformLocation("projMatrix");
    viewMatrixLoc_ = shaderProgram_->uniformLocation("viewMatrix");

    // Create and bind VAO
    vao_.reset(new QOpenGLVertexArrayObject());
    vao_->create();
    QOpenGLVertexArrayObject::Binder vaoBinder(vao_.get());

    // Create and allocate VBO
    vbo_.create();
    vbo_.bind();
    vbo_.allocate(data_.data(), data_.size() * sizeof(glm::vec2));

    // Set vertex attribute bindings
    vbo_.bind();
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0);
    vbo_.release();

    // Release shader program
    shaderProgram_->release();
}

void OpenGLWidget::resizeGL(int w, int h)
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

void OpenGLWidget::paintGL()
{
    // Clear color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Bind VAO and shader program
    QOpenGLVertexArrayObject::Binder vaoBinder(vao_.get());
    shaderProgram_->bind();

    // Set shader uniform values
    shaderProgram_->setUniformValue(projMatrixLoc_, projMatrix_);
    shaderProgram_->setUniformValue(viewMatrixLoc_, viewMatrix_);

    // Draw triangles
    glDrawArrays(GL_TRIANGLES, 0, data_.size());

    // Release Shader program
    shaderProgram_->release();
}
*/
