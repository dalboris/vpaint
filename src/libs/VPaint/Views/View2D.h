// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef VIEW2D_H
#define VIEW2D_H

#include "Views/View.h"
#include "Views/View2DMouseEvent.h"
#include "Cameras/Camera2D.h"

#include <QPointF>

#include <memory>

class View2DRenderer;
class SceneRendererSharedResources;
class SceneRenderer;

/// \class View2D
/// \brief A QWidget that displays a given Scene and provides mouse actions
/// for 2D camera navigation.
///
class View2D: public View
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(View2D)

public:
    /// Constructs a View2D displaying the given \p scene, using the
    /// given \p sceneRendererSharedResources.
    ///
    View2D(Scene * scene,
           SceneRendererSharedResources * sceneRendererSharedResources,
           QWidget * parent);

    /// Destructs this View2D.
    ///
    ~View2D();

    /// Returns the SceneRendererSharedResources associated with this View2D.
    ///
    SceneRendererSharedResources * sceneRendererSharedResources() const;

    /// Returns the camera associated with this View2D.
    ///
    Camera2D * camera() const;

    /// Returns the view coordinate \p viewPos mapped to scene coordinates.
    ///
    QPointF mapToScene(const QPointF & viewPos) const;

protected:
    /// Implements the pure virtual function OpenGLWidget::makeRenderer().
    ///
    OpenGLRenderer * makeRenderer() const;

    /// Implements the pure virtual function View::makeMouseEvent().
    ///
    View2DMouseEvent * makeMouseEvent();

private:
    void addActions_();

private:
    // Observed QObjects
    SceneRendererSharedResources * sceneRendererSharedResources_;

    // Owned DataObjects
    DataObjectPtr<Camera2D> camera2D_;
};

#endif // VIEW2D_H
