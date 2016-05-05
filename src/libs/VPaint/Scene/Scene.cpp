// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "Scene.h"

#include <glm/geometric.hpp>

Scene::Scene()
{
    data_.layers.push_back(DataObjectPtr<Layer>());
    connect(data().layers[0].get(), &Layer::changed, this, &Scene::changed);
}

Layer * Scene::activeLayer() const
{
    return data().layers[0].get();
}

Vac * Scene::activeVac() const
{
    return activeLayer()->vac();
}
