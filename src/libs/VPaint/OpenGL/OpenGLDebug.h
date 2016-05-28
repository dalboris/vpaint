// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENGLDEBUG_H
#define OPENGLDEBUG_H

#include "OpenGLFunctions.h"

#include <glm/vec2.hpp>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <vector>

class QColor;

class OpenGLDebug
{
public:
    OpenGLDebug(OpenGLFunctions * f,
                const QMatrix4x4 & projMatrix,
                const QMatrix4x4 & viewMatrix);

    void setColor(const QColor & color);

    void draw(const std::vector<glm::vec2> & vertices, GLenum mode);


private:
    // OpenGLFunctions
    OpenGLFunctions * f_;

    // Shader
    QOpenGLShaderProgram shaderProgram_;
    int vertexLoc_;
    int projMatrixLoc_;
    int viewMatrixLoc_;
    int colorLoc_;
};

#endif // OPENGLDEBUG_H
