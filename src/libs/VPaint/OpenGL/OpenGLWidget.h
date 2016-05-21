// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include "OpenGL/OpenGLFunctions.h"

#include <QOpenGLWidget>

#include <memory>

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
    /// Constructs an OpenGLWidget.
    ///
    OpenGLWidget(QWidget * parent);

    /// Destructs this OpenGLWidget, ensuring proper cleanup. You should make
    /// sure that the OpenGLRenderer is still alive at this point, since its
    /// cleanup() function will be invoked.
    ///
    virtual ~OpenGLWidget();

    OpenGLRenderer * renderer() const;
    OpenGLFunctions * functions() const;

public slots:
    void cleanup();

private slots:
    void onAboutToCompose();
    void onFrameSwapped();

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    /// Sets the OpenGLRenderer of this OpenGLWidget. Ownership is
    /// passed to the OpenGLWidget. This function must be called
    /// in the constructor of derived classes.
    ///
    void setRenderer(OpenGLRenderer * renderer);

private:
    std::unique_ptr<OpenGLRenderer> renderer_;
};

#endif // OPENGLWIDGET_H
