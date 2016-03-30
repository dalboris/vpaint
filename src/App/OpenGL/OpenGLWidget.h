// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

/*
#include "Core/Cache.h"
#include "Core/Memory.h"

#include <QOpenGLFunctions>
*/

/*
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QMatrix4x4>

#include <vector>
#include <glm/vec2.hpp>
*/

#include <QOpenGLWidget>

#include "OpenGL/OpenGLFunctions.h"

#include <QElapsedTimer>

class OpenGLRenderer;

/// \class OpenGLWidget
/// \brief A widget that sets up an OpenGL context and initialize shaders
///
/// Note: since Qt::AA_ShareOpenGLContexts is enabled, the QOpenGLContext
/// associated with this OpenGLWidget is guaranteed to never change,
/// not even when reparenting.
///
class OpenGLWidget: public QOpenGLWidget
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(OpenGLWidget)

public:
    /// Constructs an OpenGLWidget. The given \p renderer must be non-null
    /// and must outlive this OpenGLWidget.
    ///
    OpenGLWidget(QWidget * parent);

    /// Sets renderer to use. The given \p renderer must be non-null and must
    /// outlive this OpenGLWidget. This setter must be call before the first
    /// initializeGL() is issued. This OpenGLWidget does not take ownership of
    /// the renderer.
    ///
    void setRenderer(OpenGLRenderer * renderer);

    /// Destructs this OpenGLWidget, ensuring proper cleanup. You should make
    /// sure that the OpenGLRenderer is still alive at this point, since its
    /// cleanup() function will be invoked.
    ///
    virtual ~OpenGLWidget();

    OpenGLRenderer * renderer() const;
    OpenGLFunctions * functions() const;

public slots:
    void cleanup();

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

private:
    OpenGLRenderer * renderer_;
};

#endif // OPENGLWIDGET_H
