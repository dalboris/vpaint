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

struct SceneData
{
    // Canvas
    //Canvas * canvas = nullptr;

    // Layers
    std::vector<DataObjectPtr<Layer>> layers;
};

#endif // SCENEDATA_H
