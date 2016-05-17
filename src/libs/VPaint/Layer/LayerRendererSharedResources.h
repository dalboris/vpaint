// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef LAYERRENDERERSHAREDRESOURCES_H
#define LAYERRENDERERSHAREDRESOURCES_H

#include "OpenGL/OpenGLFunctions.h"

#include <QObject>

#include <memory>

class Layer;
class VacRendererSharedResources;

/// \class LayerRendererSharedResources
/// \brief A class that stores the shared resources required to render a Layer.
///
class LayerRendererSharedResources: public QObject
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(LayerRendererSharedResources)

public:
    /// Constructs a LayerRendererSharedResources for the given \p layer.
    ///
    LayerRendererSharedResources(Layer * layer);

    /// Destructs this LayerRendererSharedResources.
    ///
    ~LayerRendererSharedResources();

    /// Returns a pointer to the layer associated with these shared resources.
    ///
    Layer * layer() const;

    /// Returns a pointer to the shared resources required to render the Vac of
    /// the layer.
    ///
    VacRendererSharedResources * vacRendererSharedResources() const;

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
    // Associated layer
    Layer * layer_;

    // Shared resources of the layer's VAC
    std::unique_ptr<VacRendererSharedResources> vacRendererSharedResources_;
};

#endif // LAYERRENDERERSHAREDRESOURCES_H
