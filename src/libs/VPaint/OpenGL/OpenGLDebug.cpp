// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "OpenGLDebug.h"

#include <QColor>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

OpenGLDebug::OpenGLDebug(OpenGLFunctions * f,
                         const QMatrix4x4 & projMatrix,
                         const QMatrix4x4 & viewMatrix) :
    f_(f)

{
    // Initialize shader program
    shaderProgram_.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/OpenGL/Shaders/shadeless-unicolor.v.glsl");
    shaderProgram_.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/OpenGL/Shaders/shadeless-unicolor.f.glsl");
    shaderProgram_.link();

    // Get shader locations
    shaderProgram_.bind();
    vertexLoc_     = shaderProgram_.attributeLocation("vertex");
    projMatrixLoc_ = shaderProgram_.uniformLocation("projMatrix");
    viewMatrixLoc_ = shaderProgram_.uniformLocation("viewMatrix");
    colorLoc_      = shaderProgram_.uniformLocation("color");
    shaderProgram_.release();

    // Set matrices
    shaderProgram_.bind();
    shaderProgram_.setUniformValue(projMatrixLoc_, projMatrix);
    shaderProgram_.setUniformValue(viewMatrixLoc_, viewMatrix);
    shaderProgram_.release();

    // Set default color
    setColor(QColor(Qt::black));
}

void OpenGLDebug::setColor(const QColor & color)
{
    shaderProgram_.bind();
    shaderProgram_.setUniformValue(colorLoc_, color);
    shaderProgram_.release();
}

void OpenGLDebug::draw(const std::vector<glm::vec2> & vertices, GLenum mode)
{
    // Create VBO
    QOpenGLBuffer vbo;
    vbo.create();
    vbo.bind();
    vbo.allocate(vertices.data(), vertices.size() * sizeof(glm::vec2));
    vbo.release();

    // Create VAO
    QOpenGLVertexArrayObject vao;
    vao.create();
    vao.bind();
    vbo.bind();
    GLsizei  stride  = sizeof(glm::vec2);
    GLvoid * pointer = reinterpret_cast<void*>(0);
    f_->glEnableVertexAttribArray(vertexLoc_);
    f_->glVertexAttribPointer(
                vertexLoc_, // index of the generic vertex attribute
                2,          // number of components   (x and y components)
                GL_FLOAT,   // type of each component
                GL_FALSE,   // should it be normalized
                stride,     // byte offset between consecutive vertex attributes
                pointer);   // byte offset between the first attribute and the pointer given to allocate()
    vbo.release();
    vao.release();

    // Draw points
    shaderProgram_.bind();
    vao.bind();
    f_->glDrawArrays(
                mode,             // mode
                0,                // first index
                vertices.size()); // number of indices
    vao.release();
    shaderProgram_.release();

    // Destroy VAO
    vao.destroy();

    // Destroy VBO
    vbo.destroy();
}

void OpenGLDebug::draw(const std::vector<glm::dvec2> & vertices, GLenum mode)
{
    std::vector<glm::vec2> fvertices;
    fvertices.reserve(vertices.size());
    for (const glm::dvec2 & vertex: vertices)
        fvertices.push_back(glm::vec2(vertex)); // dvec2 -> vec2 conversion
    draw(fvertices, mode);
}
