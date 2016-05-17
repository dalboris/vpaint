// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef SCENERENDERERSHAREDRESOURCES_H
#define SCENERENDERERSHAREDRESOURCES_H

#include "OpenGL/OpenGLFunctions.h"

#include <QObject>

#include <vector>
#include <memory>

class Scene;
class LayerRendererSharedResources;

/// \class SceneRendererSharedResources
/// \brief A class that stores the shared resources required to render a Scene.
///
class SceneRendererSharedResources: public QObject
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(SceneRendererSharedResources)

public:
    /// Constructs a SceneRendererSharedResources for the given \p scene.
    ///
    SceneRendererSharedResources(Scene * scene);

    /// Destructs this SceneRendererSharedResources.
    ///
    ~SceneRendererSharedResources();

    /// Returns a pointer to the scene associated with these shared resources.
    ///
    Scene * scene() const;

    /// Returns a pointer to the shared resources required to render the
    /// i-th layer of the scene.
    ///
    LayerRendererSharedResources * layerRendererSharedResources(int i) const;

    /// Initializes the resources.
    ///
    void initialize(OpenGLFunctions * f);

    /// Updates the resources.
    ///
    void update(OpenGLFunctions * f);

    /// Cleanup the resources.
    ///
    void cleanup(OpenGLFunctions * f);

private:
    // Associated scene
    Scene * scene_;

    // Shared resources of the scene's layers
    std::vector<std::unique_ptr<LayerRendererSharedResources>> layerRendererSharedResources_;
};

#endif // SCENERENDERERSHAREDRESOURCES_H
