// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef LAYER_H
#define LAYER_H

#include "Core/DataObject.h"
#include "Layer/LayerData.h"

/// \class Layer
/// \brief A DataObject subclass that represents a layer.
///
class Layer: public DataObject<LayerData>
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(Layer)

public:
    Layer() {}

};

#endif // LAYER_H
