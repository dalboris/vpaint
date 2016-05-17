// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "Scene.h"

Scene::Scene()
{
    // Create the layer.
    // Note: for now, there is only one layer.
    data_.layers.push_back(DataObjectPtr<Layer>());

    // Emit Scene::changed() whenever a layer changes.
    for (int i=0; i<numLayers(); ++i)
    {
        connect(layer(i), &Layer::changed, this, &Scene::changed);
    }
}

int Scene::numLayers() const
{
    return data_.layers.size();
}

Layer * Scene::layer(int i) const
{
    return data().layers[i].get();
}

Layer * Scene::activeLayer() const
{
    return layer(0);
}

Vac * Scene::activeVac() const
{
    return activeLayer()->vac();
}
