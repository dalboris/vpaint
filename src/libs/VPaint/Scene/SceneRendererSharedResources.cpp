// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "Scene/SceneRendererSharedResources.h"

#include "Scene/Scene.h"
#include "Layer/LayerRendererSharedResources.h"

SceneRendererSharedResources::SceneRendererSharedResources(Scene * scene) :
    scene_(scene),
    layerRendererSharedResources_()
{
    for (int i=0; i<scene->numLayers(); ++i)
    {
        layerRendererSharedResources_.emplace_back(
                    new LayerRendererSharedResources(scene->layer(i)));
    }
}

SceneRendererSharedResources::~SceneRendererSharedResources()
{
}

Scene * SceneRendererSharedResources::scene() const
{
    return scene_;
}

LayerRendererSharedResources * SceneRendererSharedResources::layerRendererSharedResources(int i) const
{
    return layerRendererSharedResources_[i].get();
}

void SceneRendererSharedResources::initialize(OpenGLFunctions * f)
{
    for (int i=0; i<scene()->numLayers(); ++i)
    {
        layerRendererSharedResources(i)->initialize(f);
    }
}

void SceneRendererSharedResources::update(OpenGLFunctions * f)
{
    for (int i=0; i<scene()->numLayers(); ++i)
    {
        layerRendererSharedResources(i)->update(f);
    }
}

void SceneRendererSharedResources::cleanup(OpenGLFunctions * f)
{
    for (int i=0; i<scene()->numLayers(); ++i)
    {
        layerRendererSharedResources(i)->cleanup(f);
    }
}
