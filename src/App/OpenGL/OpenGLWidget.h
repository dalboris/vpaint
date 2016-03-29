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

    /// Sets renderer to use. The given \p renderer must be non-null
    /// and must outlive this OpenGLWidget. This setter must be call
    /// before the first initializeGL() is issued.
    ///
    void setRenderer(OpenGLRenderer * renderer);

    /// Destructs this OpenGLWidget, ensuring proper cleanup. It is important
    /// that the OpenGLRenderer (e.g., of derived type View2DRenderer), is
    /// still alive when this destructor is called. This means that indirectly,
    /// it is also important that SceneRenderer is still alive.
    ///
    /// Example of what happens:
    ///   - ~View2D() calls ~View()
    ///   - ~View() calls ~OpenGLWidget()
    ///   - ~OpenGLWidget() calls OpenGLWidget::cleanup()
    ///   - OpenGLWidget::cleanup() calls renderer()->cleanup(f);
    ///   - View2DRenderer::cleanup(f) calls sceneRenderer()->cleanup(f);
    ///   - SceneRenderer::cleanup(f) does its things.
    ///
    /// In particular, this means that the Scene object, and the SceneRenderer
    /// objects can't be QObject children of MainWindow. Otherwise, since the
    /// View2D is also a children of MainWindow (no choice as a subwidget),
    /// then View2D and Scene/SceneRenderer will be deleted in an unspecified
    /// order.
    ///
    /// Also, the Scene/SceneRenderer can't be explicitely deleted in
    /// ~MainWindow, since the View2D will be deleted in ~QObject, *after*
    /// ~MainWindow.
    ///
    /// One option can be to delete explicitely all the views (View2D, View3D)
    /// in ~MainWindow, but that's ugly.
    ///
    /// Conclusion: the only clean solutions are:
    ///   1. construct and destruct Scene/SceneRenderer outside of MainWindow, or
    ///   2. not call cleanup in ~OpenGLWidget(), i.e. rely on client to call it.
    ///
    /// XXX In any case, it is still necessary to design a proper system so
    ///     that SceneRenderer::cleanup() is only called when the last View is
    ///     destructed.
    ///
    virtual ~OpenGLWidget();

    OpenGLRenderer * renderer() const;
    OpenGLFunctions * functions() const;


    /*

    const QMatrix4x4 & projectionMatrix() const;
    const QMatrix4x4 & projectionMatrixInverse() const;
    void setProjectionMatrix(const QMatrix4x4 & projectionMatrix);

    const QMatrix4x4 & viewMatrix() const;
    const QMatrix4x4 & viewMatrixInverse() const;
    void setViewMatrix(const QMatrix4x4 & viewMatrix);

    */

public slots:
    void cleanup();

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

private:
    OpenGLRenderer * renderer_;
    QElapsedTimer elapsedTimer_;


    /*

    // Data to render
    std::vector<glm::vec2> data_;

    // Projection and view matrices
    QMatrix4x4 projMatrix_;
    QMatrix4x4 viewMatrix_;
    mutable Cache<QMatrix4x4> projMatrixInv_;
    mutable Cache<QMatrix4x4> viewMatrixInv_;

    // GPU resources
    QOpenGLBuffer vbo_;
    UniquePtr<QOpenGLVertexArrayObject> vao_;
    UniquePtr<QOpenGLShaderProgram> shaderProgram_;

    // Shader uniform locations
    int projMatrixLoc_;
    int viewMatrixLoc_;
    */
};

#endif // OPENGLWIDGET_H
