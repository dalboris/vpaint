// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "LayerRendererSharedResources.h"

#include "Layer/Layer.h"
#include "Vac/VacRendererSharedResources.h"

LayerRendererSharedResources::LayerRendererSharedResources(Layer * layer) :
    layer_(layer),
    vacRendererSharedResources_(new VacRendererSharedResources(layer->vac()))
{
}

LayerRendererSharedResources::~LayerRendererSharedResources()
{
}

Layer * LayerRendererSharedResources::layer() const
{
    return layer_;
}

VacRendererSharedResources * LayerRendererSharedResources::vacRendererSharedResources() const
{
    return vacRendererSharedResources_.get();
}

void LayerRendererSharedResources::initialize(OpenGLFunctions * f)
{
    vacRendererSharedResources()->initialize(f);
}

void LayerRendererSharedResources::update(OpenGLFunctions * f)
{
    vacRendererSharedResources()->update(f);
}

void LayerRendererSharedResources::cleanup(OpenGLFunctions * f)
{
    vacRendererSharedResources()->cleanup(f);
}
