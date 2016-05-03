// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef VIEW_H
#define VIEW_H

#include "Core/Memory.h"
#include "OpenGL/OpenGLWidget.h"
#include "Views/ViewMouseEvent.h"
#include "Views/ViewMouseAction.h"

#include <vector>

class Scene;
class Timeline;

/// \class View
/// \brief A subclass of OpenGLWidget that provides mouse event management
/// and connection with a scene.
///
/// This class listens to the low-level Qt mouse events and provides
/// higher-level event handling from them. By design, it does not handle
/// multiple clicks.
///
/// Also, every View observes a Scene, which must be a valid non-null
/// pointer. The given Scene must outlive the View, and it is not possible
/// to change which Scene the View is observing. Though, you can clear()
/// the Scene and fill it up with completely new data.
///
/// Each time the Scene changes (i.e., emits the changed() signal), the
/// OpenGLWidget::update() function is called.///
///
class View: public OpenGLWidget
{
private:
    Q_OBJECT
    Q_DISABLE_COPY(View)

public:
    /// Construct a View observing the given \p scene.
    ///
    View(Scene * scene, QWidget * parent);

    /// Returns the observed Scene
    ///
    Scene * scene() const;

protected:
    /// Adds a ViewMouseAction to the view. The View takes ownership of the
    /// action.
    ///
    void addMouseAction(ViewMouseAction * action);

    /// Must be implemented by derived classes. Must allocate a new
    /// ViewMouseEvent and returns it. Ownership is transferred to
    /// caller. The pointer will be used polymorphically.
    ///
    /// The reason for polymorphism is that View2D and View3D store
    /// different information within a ViewMouseEvent. For instance,
    /// in addition to viewPos(), a View2D stores a scenePos() -the
    /// 2D position of the mouse in scene coordinates- and a View3D
    /// stores a sceneRay() -the ray spanning all possible 3D positions
    /// of the mouse in space-time coordinates-.
    ///
    virtual ViewMouseEvent * makeMouseEvent() = 0;

protected slots:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    // Helper methods
    void updateViewOnSceneChange_();

private:
    // Child QObjects
    // (none)

    // Observed QObjects
    Scene * scene_;

    // List of available actions
    std::vector<UniquePtr<ViewMouseAction>> mouseActions_;

    // Clic action and PMR action selected among available actions
    ViewMouseAction * mouseClickAction_ = nullptr;
    ViewMouseAction * mousePMRAction_ = nullptr;

    // Generated mouse event
    UniquePtr<ViewMouseEvent> mouseEvent_;
};

#endif // VIEW_H
