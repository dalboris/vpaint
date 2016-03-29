// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "Scene/Scene.h"
#include "Scene/SceneRenderer.h"

#include <QtDebug>

SceneRenderer::SceneRenderer(Scene * scene,
                             QObject * parent) :
    QObject(parent),
    scene_(scene)
{

}

SceneRenderer::~SceneRenderer()
{
    qDebug() << "SceneRenderer::~SceneRenderer()";
}

Scene * SceneRenderer::scene() const
{
    return scene_;
}

void SceneRenderer::initialize(OpenGLFunctions * f)
{
    // Set clear color
    glClearColor(1, 1, 1, 1);

    // Create shader program
    shaderProgram_.reset(new QOpenGLShaderProgram());
    shaderProgram_->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/OpenGL/Shaders/Helloworld.v.glsl");
    shaderProgram_->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/OpenGL/Shaders/Helloworld.f.glsl");
    shaderProgram_->link();

    // Get shader locations
    shaderProgram_->bind();
    vertexLoc_     = shaderProgram_->attributeLocation("vertex");
    projMatrixLoc_ = shaderProgram_->uniformLocation("projMatrix");
    viewMatrixLoc_ = shaderProgram_->uniformLocation("viewMatrix");
    shaderProgram_->release();

    // Create VBO
    vbo_.create();

    // Create VAO
    vao_.reset(new QOpenGLVertexArrayObject());
    vao_->create();

    // Store attribute bindings in VAO
    GLsizei  stride  = sizeof(SceneDataSample) / 2;
    GLvoid * pointer = reinterpret_cast<void*>(offsetof(SceneDataSample, leftBoundary));
    vao_->bind();
    vbo_.bind();
    f->glEnableVertexAttribArray(vertexLoc_);
    f->glVertexAttribPointer(
                vertexLoc_, // index of the generic vertex attribute
                2,          // number of components   (x and y components)
                GL_FLOAT,   // type of each component
                GL_FALSE,   // should it be normalized
                stride,     // byte offset between consecutive vertex attributes
                pointer);   // byte offset between the first attribute and the pointer given to allocate()
    vbo_.release();
    vao_->release();
}

void SceneRenderer::render2D(OpenGLFunctions * f, const QMatrix4x4 & projMatrix, const QMatrix4x4 & viewMatrix)
{
    // Clear color and depth buffer
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Send data to GPU
    // XXX TODO: do it only if data changed
    std::vector<SceneDataSample> & samples = scene()->data_.samples;
    vbo_.bind();
    vbo_.allocate(samples.data(), samples.size() * sizeof(SceneDataSample));
    vbo_.release();

    // Bind shader program
    shaderProgram_->bind();

    // Set uniform values
    shaderProgram_->setUniformValue(projMatrixLoc_, projMatrix);
    shaderProgram_->setUniformValue(viewMatrixLoc_, viewMatrix);

    // Draw triangles
    vao_->bind();
    f->glDrawArrays(GL_TRIANGLE_STRIP,   // mode
                    0,                   // starting index
                    samples.size() * 2); // number of indices
    vao_->release();

    // Release shader program
    shaderProgram_->release();
}

void SceneRenderer::render3D(OpenGLFunctions * /*f*/)
{
    // XXX TODO
}

void SceneRenderer::cleanup(OpenGLFunctions * /*f*/)
{
    //vbo_.destroy(); // XXX SegFault. Why? Maybe because allocate wasn't call?
    vao_.reset();
    shaderProgram_.reset();
}
