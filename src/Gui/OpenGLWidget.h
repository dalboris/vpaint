// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>

#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QMatrix4x4>

#include <vector>
#include <glm/vec2.hpp>

class OpenGLWidget: public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    OpenGLWidget(QWidget * parent);
    ~OpenGLWidget();

public slots:
    void cleanup();

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

private:
    // Data to render
    std::vector<glm::vec2> data_;

    // Projection and view matrices
    QMatrix4x4 projMatrix_;
    QMatrix4x4 viewMatrix_;

    // GPU resources
    QOpenGLVertexArrayObject vao_;
    QOpenGLBuffer vbo_;
    QOpenGLShaderProgram *shaderProgram_;

    // Shader uniform locations
    int projMatrixLoc_;
    int viewMatrixLoc_;
};

#endif // OPENGLWIDGET_H
