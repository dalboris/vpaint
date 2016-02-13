// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "OpenGLWidget.h"

static const char *vertexShaderSource =
    "#version 150\n"
    "in vec4 vertex;\n"
    "uniform mat4 projMatrix;\n"
    "uniform mat4 viewMatrix;\n"
    "void main() {\n"
    "   gl_Position = projMatrix * viewMatrix * vertex;\n"
    "}\n";

static const char *fragmentShaderSource =
    "#version 150\n"
    "out highp vec4 fragColor;\n"
    "void main() {\n"
    "   fragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
    "}\n";

OpenGLWidget::OpenGLWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    shaderProgram_(0)
{
    data_.push_back(glm::vec2(50, 50));
    data_.push_back(glm::vec2(50, 100));
    data_.push_back(glm::vec2(100, 50));
}

OpenGLWidget::~OpenGLWidget()
{
    cleanup();
}

void OpenGLWidget::cleanup()
{
    makeCurrent();
    vbo_.destroy();
    delete shaderProgram_;
    shaderProgram_ = 0;
    doneCurrent();
}

void OpenGLWidget::initializeGL()
{
    // Ensure proper cleanup
    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &OpenGLWidget::cleanup);

    // Initialize OpenGL functions
    initializeOpenGLFunctions();

    // Set clear color
    glClearColor(1, 1, 1, 1);

    // Allocate shader program
    shaderProgram_ = new QOpenGLShaderProgram;
    shaderProgram_->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    shaderProgram_->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    shaderProgram_->bindAttributeLocation("vertex", 0);
    shaderProgram_->link();

    // Bind shader program
    shaderProgram_->bind();

    // Get shader uniform locations
    projMatrixLoc_ = shaderProgram_->uniformLocation("projMatrix");
    viewMatrixLoc_ = shaderProgram_->uniformLocation("viewMatrix");

    // Create VAO
    vao_.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&vao_);

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
    projMatrix_.setToIdentity();
    projMatrix_.ortho(0.0f, (float)w, (float)h, 0.0f, -1.0f, 1.0f);
}

void OpenGLWidget::paintGL()
{
    // Clear color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set view matrix
    viewMatrix_.setToIdentity();

    // Bind VAO and shader program
    QOpenGLVertexArrayObject::Binder vaoBinder(&vao_);
    shaderProgram_->bind();

    // Set shader uniform values
    shaderProgram_->setUniformValue(projMatrixLoc_, projMatrix_);
    shaderProgram_->setUniformValue(viewMatrixLoc_, viewMatrix_);

    // Draw triangles
    glDrawArrays(GL_TRIANGLES, 0, data_.size());

    // Release Shader program
    shaderProgram_->release();
}
