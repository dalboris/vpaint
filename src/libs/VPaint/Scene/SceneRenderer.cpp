// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "SceneRenderer.h"

#include "Scene/Scene.h"
#include "Scene/SceneRendererSharedResources.h"
#include "Layer/LayerRenderer.h"

SceneRenderer::SceneRenderer(SceneRendererSharedResources * sharedResources) :
    sharedResources_(sharedResources),
    layerRenderers_()
{
    for (int i=0; i<scene()->numLayers(); ++i)
    {
        layerRenderers_.emplace_back(
                    new LayerRenderer(
                        sharedResources->layerRendererSharedResources(i)));
    }
}

SceneRenderer::~SceneRenderer()
{
}

SceneRendererSharedResources * SceneRenderer::sharedResources() const
{
    return sharedResources_;
}

LayerRenderer * SceneRenderer::layerRenderer(int i) const
{
    return layerRenderers_[i].get();
}

Scene * SceneRenderer::scene() const
{
    return sharedResources()->scene();
}

void SceneRenderer::initialize(OpenGLFunctions * f)
{
    // Initialize shared resources
    sharedResources()->initialize(f);

    // Initialize layer renderers
    for (int i=0; i<scene()->numLayers(); ++i)
    {
        layerRenderer(i)->initialize(f);
    }
}

void SceneRenderer::render2D(OpenGLFunctions * f, const QMatrix4x4 & projMatrix, const QMatrix4x4 & viewMatrix)
{
    // Update shared resources
    sharedResources()->update(f);

    // Render layers
    for (int i=0; i<scene()->numLayers(); ++i)
    {
        layerRenderer(i)->render2D(f, projMatrix, viewMatrix);
    }
}

void SceneRenderer::render3D(OpenGLFunctions * /*f*/)
{
    // XXX TODO
}

void SceneRenderer::cleanup(OpenGLFunctions * f)
{
    // Cleanup shared resources
    sharedResources()->cleanup(f);

    // Cleanup layer renderers
    for (int i=0; i<scene()->numLayers(); ++i)
    {
        layerRenderer(i)->cleanup(f);
    }
}
