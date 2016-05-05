// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef SCENE_H
#define SCENE_H

#include "Core/DataObject.h"
#include "Scene/SceneData.h"

/// \class Scene
/// \brief A class to represent the whole scene.
///
class Scene: public DataObject<SceneData>
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(Scene)

public:
    /// Constructs a Scene.
    ///
    Scene();

    /// Returns the active layer of this Scene. There is always a
    /// non-null active layer, so this never returns nullptr.
    ///
    Layer * activeLayer() const;

    /// Returns the Vac of the active layer of this Scene. This never returns
    /// nullptr.
    ///
    Vac * activeVac() const;
};

#endif // SCENE_H
