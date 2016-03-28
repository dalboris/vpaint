// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef VIEW2DRENDERER_H
#define VIEW2DRENDERER_H

#include "OpenGL/OpenGLRenderer.h"

class OpenGLSharedResources;
class SceneRenderer;

/// \class View2DRenderer
/// \brief The renderer object owned by each View2D.
///
class View2DRenderer: public OpenGLRenderer
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(View2DRenderer)

public:
    /// Constructs a View2DRenderer.
    ///
    View2DRenderer(SceneRenderer * sceneRenderer,
                   QObject * parent);

    /// Returns the SceneRenderer of this View2DRenderer.
    ///
    SceneRenderer * sceneRenderer();

    /// Implements OpenGLRenderer::initialize().
    ///
    virtual void initialize(OpenGLFunctions * f);

    /// Implements OpenGLRenderer::resize().
    ///
    virtual void resize(OpenGLFunctions * f, int w, int h);

    /// Implements OpenGLRenderer::render().
    ///
    virtual void render(OpenGLFunctions * f);

    /// Implements OpenGLRenderer::cleanup().
    ///
    virtual void cleanup(OpenGLFunctions * f);

private:
    SceneRenderer * sceneRenderer_;
};

#endif // VIEW2DRENDERER_H
