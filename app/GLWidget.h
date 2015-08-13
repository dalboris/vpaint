// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

/*
 *  GLWidget
 *
 *  This  class  provides a  convenient  OpenGL  View with  predefined
 *  behaviours, such as:
 *    - enabling the depth test, phong shading, and blending
 *    - initializing the projection and modelview matrices
 *    - providing a convenient camera, with Maya controls
 *    - providing predefined materials
 *    - providing predefined lighting
 *    - can display an horizontal grid at y=0
 *    - providing drawing functions
 *
 *  To  use it,  the best  is  to inherit  from it,  and overload  the
 *  virtual    functions.   You    can   change    interactively   (or
 *  programmatically)  the  default  behaviour  by  using  the  widget
 *  DisplaySettings which  is created  alongside the GLWidget,  and that
 *  you could put somewhere in your application
 *
 */
 
#ifndef GLWIDGET_H
#define GLWIDGET_H

#include "OpenGL.h"
#include <QGLWidget>

#include "GLWidget_Camera.h"
#include "GLWidget_Camera2D.h"
#include "GLWidget_Light.h"
#include "GLWidget_Material.h"
#include "GLWidget_Settings.h"
#include <QMouseEvent>
#include <QElapsedTimer>

class GLUtils;

class GLAction
{
public:
    // No action
    static const int None = 0;

    // 3D navigation
    static const int Dolly = 1;
    static const int Travelling = 2;
    static const int Zoom = 3;
};

class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLWidget(QWidget *parent, bool isOnly2D = false);
    virtual ~GLWidget();

    // Return if a mouse action is being performed
    bool isBusy();
    
    // Get or set the current camera
    GLWidget_Camera camera() const;
    GLWidget_Camera2D camera2D() const;
    void setCamera(const GLWidget_Camera & newCamera);
    void setCamera2D(const GLWidget_Camera2D & newCamera);
    void setOrthographic(bool isOrtho);

    // get current size of viewport
    double viewportHeight();
    double viewportWidth();

    // enable or disable camera control
    void enableCameraMovement(bool value);

    // get GLWidget settings as a widget
    GLWidget_Settings * settings();

signals:
    // called when the camera is moved. In practice,
    // used by derived classes implementing picking,
    // to know when to recompute picking-related data.
    void viewIsGoingToChange(int x, int y);
    void viewIsBeingChanged(int x, int y);
    void viewChanged(int x, int y);

    void viewResized();

    void mousePressed(GLWidget * w);
    void mouseMoved(GLWidget * w);
    void mouseReleased(GLWidget * w);
    void mouseEntered(GLWidget * w);
    void mouseLeft(GLWidget * w);

protected:
    // Low-level mouse/tablet events handling.
    // By design, do not handle multiple clics
    // On MousePress (or equivalent tablet event), calls:
    //      - decideAction(x,y)
    // On MousePress,Move,Release (or equivalent tablet event), calls the high-level:
    //      - MoveEvent(x,y): mouse moved with no action
    //      - ClicEvent(action,x,y): a one-shot press-release mouse action.
    //                               Might be called on press or on release, depending if
    //                               There is ambiguity with a PMR action or not
    //      - PMR[...](action,x,y): a Press-Move-Release mouse action

    // Original mouse/tablet events
protected slots:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void tabletEvent(QTabletEvent * event);
    void enterEvent(QEvent * event);
    void leaveEvent(QEvent * event);
protected:
    // Delegated to these methods for consistency
    void delegateMousePress(QMouseEvent *event);
    void delegateMouseMove(QMouseEvent *event);
    void delegateMouseRelease(QMouseEvent *event);

    // High-level mouse/tablet action handling.
    // Derived classes can use the following member
    // variables to decide what to do:
    //
    //    bool mouse_LeftButton_
    //    bool mouse_MidButton_
    //    bool mouse_RightButton_
    //
    //    bool mouse_AltWasDown_
    //    bool mouse_ControlWasDown_
    //    bool mouse_ShiftWasDown_
    //
    //    int mouse_PressEvent_X
    //    int mouse_PressEvent_Y
    //    int mouse_Event_X
    //    int mouse_Event_Y
    //
    //    double mouse_Event_XScene_;
    //    double mouse_Event_YScene_;
    //    double mouse_PressEvent_XScene_;
    //    double mouse_PressEvent_YScene_;
    //
    //    bool mouse_isTablet_;
    //    double mouse_tabletPressure_;
    //
    virtual int decideClicAction();
    virtual int decidePMRAction();
    virtual void MoveEvent(double x, double y);
    virtual void ClicEvent(int action, double x, double y);
    virtual void PMRPressEvent(int action, double x, double y);
    virtual void PMRMoveEvent(int action, double x, double y);
    virtual void PMRReleaseEvent(int action, double x, double y);

protected slots:
    // Wheel events
    virtual void wheelEvent(QWheelEvent *event);

    // Keyboard events
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);

public:
    void rotateViewRight();
    void rotateViewLeft();
    void rotateViewUp();
    void rotateViewDown();
    void zoomIn();
    void zoomOut();

protected slots:
    // OpenGL drawing
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);

    // Reimplement this one in derived classes
    virtual void drawScene();

      
protected:
    // Member variables
    bool isOnly2D_;
    bool cameraDollyIsEnabled_;
    bool cameraTravellingIsEnabled_;
    bool cameraZoomIsEnabled_;
    GLWidget_Settings * settings_;

    // Handling Mouse Event
    // At most one of these is true at one time:
    bool mouse_LeftButton_;
    bool mouse_MidButton_;
    bool mouse_RightButton_;
    // The modifiers keys when the mousePressEvent occured
    bool mouse_AltWasDown_;
    bool mouse_ControlWasDown_;
    bool mouse_ShiftWasDown_;
    // The position when the mousePressEvent occured (in window coords)
    int mouse_Event_X_;
    int mouse_Event_Y_;
    int mouse_PressEvent_X_;
    int mouse_PressEvent_Y_;
    // Determines if  the action is  a "clic" or a press->move->release
    int mouse_ClicAction_;
    int mouse_PMRAction_;
    // Handing pen tablets
    bool mouse_isTablet_;
    double mouse_tabletPressure_;
    bool mouse_tabletPressJustReceived_;
    bool mouse_tabletReleaseJustReceived_;
    // The position when the mousePressEvent occured (in scene coords, only in 2D mode)
    double mouse_Event_XScene_;
    double mouse_Event_YScene_;
    double mouse_PressEvent_XScene_;
    double mouse_PressEvent_YScene_;
    // When the mouse cursor should be hidden
    bool mouse_HideCursor_;
    // Measure time
    QElapsedTimer mouse_timer_;
    QElapsedTimer mouse_timerIdleTime_;
    QElapsedTimer mouse_timerComputationTime_;
    // Print mouse/tablet info to the console
    bool mouse_debug_;

    // Viewport size
    double viewportWidth_;
    double viewportHeight_;

    // Autocenter behaviour
    bool autoCenterScene_;

    // 3D Navigation
    bool isOrtho_;
    GLWidget_Camera camera_;
    GLWidget_Camera camera_beforeMousePress_;

    // 2D Navigation
    GLWidget_Camera2D camera2D_;
    GLWidget_Camera2D camera2D_beforeMousePress_;

    // Set OpenGL Projection and Model/View Matrix
    void setCameraPositionAndOrientation();
      
    // Draw a useful grid
    void drawGrid();
     
    // Lighting
    int GL_LIGHT_[8];
    void setLighting();
    QList<GLWidget_Light> lights_;

    // Materials
    void setMaterial(const GLWidget_Material & m);
    GLWidget_Material material_;

    // Performance measures
    QElapsedTimer fpstimer_;
    int fpstimerCount_;

private:
    friend class GLUtils;
    static GLWidget * currentGLWidget_;
    QMouseEvent mouse_tabletEventToMouseEvent_;
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

#endif
