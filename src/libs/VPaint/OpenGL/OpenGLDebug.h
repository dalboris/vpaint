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

/// \class OpenGLDebug
/// \brief A convenient class to perform basic rendering, useful for debugging.
///
/// Example usage, to visualize a polyline as o---o----o-------o:
///
/// \code
/// // Create vertices
/// std::vector<glm::vec2> vertices;
/// vertices.push_back(0.0, 0.0);
/// vertices.push_back(0.0, 100.0);
/// vertices.push_back(50.0, 50.0);
/// vertices.push_back(100.0, 100.0);
///
/// // Draw vertices
/// OpenGLDebug glDebug;
/// glDebug.setColor(Qt::red);
/// glLineWidth(1.0);
/// glPointSize(5.0);
/// glDebug.draw(vertices, GL_LINE_STRIP);
/// glDebug.draw(vertices, GL_POINTS);
/// \endcode
///
class OpenGLDebug
{
public:
    OpenGLDebug(OpenGLFunctions * f,
                const QMatrix4x4 & projMatrix,
                const QMatrix4x4 & viewMatrix);

    /// Set the color with which to draw given vertices.
    ///
    void setColor(const QColor & color);

    /// Draws the given \p vertices with the given \p mode, with the color
    /// previously set by setColor(), without shading. Example of modes (see
    /// OpenGL documentation of glDrawArrays for exhaustive list):
    ///
    /// GL_POINTS, GL_LINE_STRIP, GL_LINE_LOOP, GL_LINES, GL_TRIANGLE_STRIP,
    /// GL_TRIANGLE_FAN, GL_TRIANGLES
    ///
    /// Under the hood, this function creates a VBO/VAO pair, sends the
    /// vertices data to VBO, calls glDrawArrays with the given mode, then
    /// destroys the VBO/VAO.
    ///
    void draw(const std::vector<glm::vec2> & vertices, GLenum mode);

    /// Convenient overload of draw(). This function first creates a deep copy
    /// of the std::vector<glm::dvec2> into a std::vector<glm::vec2>,
    /// converting all doubles to floats.
    ///
    void draw(const std::vector<glm::dvec2> & vertices, GLenum mode);

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
