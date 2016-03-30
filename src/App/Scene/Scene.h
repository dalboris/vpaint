// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef SCENE_H
#define SCENE_H

#include "Scene/SceneData.h"

#include <QObject>

class Scene: public QObject
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(Scene)

public:
    Scene(QObject * parent);

    // XXX TEMP
    void beginStroke(const glm::vec2 & centerline);
    void continueStroke(const glm::vec2 & centerline);
    void endStroke();
    const std::vector<SceneDataSample> & samples();

signals:
    void changed();

private:
    SceneData data_;

    // XXX TEMP
    size_t size() const;
    void clear();
    void addSample(const glm::vec2 & centerline);
};

#endif // SCENE_H
