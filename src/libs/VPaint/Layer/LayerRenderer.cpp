// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "LayerRenderer.h"

#include "Layer/Layer.h"
#include "Layer/LayerRendererSharedResources.h"
#include "Vac/VacRenderer.h"

LayerRenderer::LayerRenderer(LayerRendererSharedResources * sharedResources) :
    sharedResources_(sharedResources),
    vacRenderer_(new VacRenderer(sharedResources->vacRendererSharedResources()))
{
}

LayerRenderer::~LayerRenderer()
{
}

LayerRendererSharedResources * LayerRenderer::sharedResources() const
{
    return sharedResources_;
}

VacRenderer * LayerRenderer::vacRenderer() const
{
    return vacRenderer_.get();
}

Layer * LayerRenderer::layer() const
{
    return sharedResources()->layer();
}

void LayerRenderer::initialize(OpenGLFunctions * f)
{
    sharedResources()->initialize(f);
    vacRenderer()->initialize(f);
}

void LayerRenderer::render2D(OpenGLFunctions * f, const QMatrix4x4 & projMatrix, const QMatrix4x4 & viewMatrix)
{
    // Update shared resources
    sharedResources()->update(f);

    // Render VAC
    vacRenderer()->render2D(f, projMatrix, viewMatrix);
}

void LayerRenderer::render3D(OpenGLFunctions * /*f*/)
{
    // XXX TODO
}

void LayerRenderer::cleanup(OpenGLFunctions * f)
{
    sharedResources()->cleanup(f);
    vacRenderer()->cleanup(f);
}
