// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef SCENEDATA_H
#define SCENEDATA_H

#include "Layer/Layer.h"

#include <vector>
#include <glm/vec2.hpp>

struct SceneDataSample
{
    glm::vec2 centerline1;   // actual data
    glm::vec2 normal1;       // computed normal
    glm::vec2 leftBoundary;  // computed edge boundary (after join style + warping)

    glm::vec2 centerline2;   // == centerline1 (duplicated for shader)
    glm::vec2 normal2;       // == normal1     (duplicated for shader)
    glm::vec2 rightBoundary; // computed edge boundary (after join style + warping)
};

struct SceneData
{
    /*
    // Canvas
    Canvas * canvas = nullptr;
    */

    // Layers
    //std::vector<DataObjectPtr<Layer>> layers;
    DataObjectPtr<Layer> layers;

    // XXX TEMP for experimentation
    std::vector<SceneDataSample> samples;
    std::vector<float> arclengths;
    std::vector<glm::vec2> tangents;
};

#endif // SCENEDATA_H
