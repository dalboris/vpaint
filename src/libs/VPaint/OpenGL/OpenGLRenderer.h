// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENGLRENDERER_H
#define OPENGLRENDERER_H

#include <QObject>

#include "Core/Cache.h"
#include "OpenGL/OpenGLFunctions.h"

#include <QMatrix4x4>

/// \class OpenGLRenderer
/// \brief A class to render graphical data via OpenGL calls.
///
///
class OpenGLRenderer: public QObject
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(OpenGLRenderer)

public:
    /********************** Constructor / Destructor *************************/

    /// Constructs an OpenGLRenderer.
    ///
    OpenGLRenderer() {}

    /// Destructs this OpenGLRenderer. Note it this does *not* call cleanup().
    /// Indeed, cleanup() cannot be called in the destructor because:
    ///   - cleanup() is a virtual function, and won't be dispatched to the
    ///     appropriate derived implementation when called in the destructor.
    ///   - It requires an OpenGLFunctions argument which is not known by the
    ///     destructor.
    ///
    /// Therefore, it is the responsibility of the owner of this OpenGLRenderer
    /// to appropriately call initialize(), render(), and cleanup(), in the
    /// correct order.
    ///
    virtual ~OpenGLRenderer() {}


    /******************* Virtual functions to reimplement ********************/

    /// Initializes OpenGL resources of this renderer. This pure virtual
    /// function must be implemented in derived classes. It is the
    /// responsibility of the caller to ensure that the appropriate OpenGL
    /// context is current.
    ///
    virtual void initialize(OpenGLFunctions * f)=0;

    /// Updates OpenGL resources dependent on widget size. This pure virtual
    /// function must be implemented in derived classes. It is the
    /// responsibility of the caller to ensure that the appropriate OpenGL
    /// context is current.
    ///
    virtual void resize(OpenGLFunctions * f, int w, int h)=0;

    /// Performs the actual OpenGL rendering. This pure virtual function must
    /// be implemented in derived classes. It is the responsibility of the
    /// caller to ensure that the appropriate OpenGL context is current.
    ///
    virtual void render(OpenGLFunctions * f)=0;

    /// Performs cleanup of OpenGL resources. This pure virtual function must
    /// be implemented in derived classes. It is the responsibility of the
    /// caller to ensure that context() is the current OpenGL context.
    ///
    virtual void cleanup(OpenGLFunctions * f)=0;
};

#endif // OPENGLRENDERER_H
