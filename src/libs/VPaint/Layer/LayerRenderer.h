// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef LAYERRENDERER_H
#define LAYERRENDERER_H

#include "OpenGL/OpenGLFunctions.h"

#include <QObject>
#include <QMatrix4x4>

#include <memory>

class Layer;
class LayerRendererSharedResources;
class VacRenderer;

class LayerRenderer: public QObject
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(LayerRenderer)

public:
    LayerRenderer(LayerRendererSharedResources * sharedResources);
    ~LayerRenderer();

    LayerRendererSharedResources * sharedResources() const;
    VacRenderer * vacRenderer() const;

    Layer * layer() const;

    void initialize(OpenGLFunctions * f);
    void render2D(OpenGLFunctions * f, const QMatrix4x4 & projMatrix, const QMatrix4x4 & viewMatrix);
    void render3D(OpenGLFunctions * f);
    void cleanup(OpenGLFunctions * f);

private:
    // Shared resources
    LayerRendererSharedResources * sharedResources_;

    // Vac renderer
    std::unique_ptr<VacRenderer> vacRenderer_;
};

#endif // LAYERRENDERER_H
