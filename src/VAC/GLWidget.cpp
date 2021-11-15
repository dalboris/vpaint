// Copyright (C) 2012-2019 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <iostream>
#include <QtDebug>

#include "GLWidget.h"
#include "GLUtils.h"
#include <cmath>
#include <limits>
#include <QApplication>
#include "Global.h"

#define MIN_SIZE_DRAWING 5
#define GLWIDGET_PI 3.1415926535897932

GLWidget::GLWidget(QWidget *parent, bool isOnly2D) :

    QOpenGLWidget(parent),

    gl_(nullptr),

    isOnly2D_(isOnly2D),
    
    cameraDollyIsEnabled_(true),
    cameraTravellingIsEnabled_(true),
    cameraZoomIsEnabled_(true),
    
    mouse_LeftButton_(false),
    mouse_MidButton_(false),
    mouse_RightButton_(false),
    mouse_AltWasDown_(false),
    mouse_ControlWasDown_(false),
    mouse_ShiftWasDown_(false),
    mouse_PressEvent_X_(0),
    mouse_PressEvent_Y_(0),
    mouse_ClicAction_(0),
    mouse_PMRAction_(0),
    mouse_isTablet_(false),
    mouse_tabletPressure_(0),
    mouse_tabletPressJustReceived_(false),
    mouse_tabletReleaseJustReceived_(false),
    mouse_HideCursor_(false),

    autoCenterScene_(true),

    isOrtho_(false),
    camera_(),
    camera_beforeMousePress_(),
    camera2D_(),
    camera2D_beforeMousePress_(),

    lights_(),
    material_(),

    fpstimer_(),
    fpstimerCount_(0),

    mouse_tabletEventToMouseEvent_(QEvent::MouseButtonPress, QPoint(), Qt::LeftButton, Qt::NoButton, Qt::NoModifier)
{
    // To grab keyboard focus when user clicks
    setFocusPolicy(Qt::ClickFocus);

    setMinimumSize(200, 200);
    //setAutoFillBackground(false);

    // Making  mouse  move events  occur  even  without any  buttons
    // pressed. Useful for picking points
    setMouseTracking(true);

    // lighting initialisation
    GL_LIGHT_[0] = GL_LIGHT0;
    GL_LIGHT_[1] = GL_LIGHT1;
    GL_LIGHT_[2] = GL_LIGHT2;
    GL_LIGHT_[3] = GL_LIGHT3;
    GL_LIGHT_[4] = GL_LIGHT4;
    GL_LIGHT_[5] = GL_LIGHT5;
    GL_LIGHT_[6] = GL_LIGHT6;
    GL_LIGHT_[7] = GL_LIGHT7;
}


GLWidget::~GLWidget()
{

}


GLWidget_Camera GLWidget::camera() const
{
    return camera_;
}

void GLWidget::setCamera(const GLWidget_Camera &newCamera)
{
    camera_ = newCamera;
}

GLWidget_Camera2D GLWidget::camera2D() const
{
    return camera2D_;
}

void GLWidget::setCamera2D(const GLWidget_Camera2D &newCamera)
{
    camera2D_ = newCamera;
}

void GLWidget::setOrthographic(bool isOrtho)
{
    isOrtho_ = isOrtho; update();
}

void GLWidget::enableCameraMovement(bool value)
{
    cameraDollyIsEnabled_ = value;
    cameraTravellingIsEnabled_ = value;
    cameraZoomIsEnabled_ = value;
}

bool GLWidget::isBusy()
{
    return mouse_ClicAction_ || mouse_PMRAction_;
}

/*********************************************************
 *                     Actions 
 */


void GLWidget::rotateViewRight()
{
    if(cameraDollyIsEnabled_)
    {
        camera_.setPhi(  camera_.phi() + GLWIDGET_PI/24 * 0.1);
        update();
    }
}
void GLWidget::rotateViewLeft()
{
    if(cameraDollyIsEnabled_)
    {
        camera_.setPhi(  camera_.phi() - GLWIDGET_PI/24 * 0.1);
        update();
    }    
}
void GLWidget::rotateViewUp()
{
    if(cameraDollyIsEnabled_)
    {
        camera_.setTheta(  camera_.theta() + GLWIDGET_PI/24 );
        update();
    }
}
void GLWidget::rotateViewDown()
{
    if(cameraDollyIsEnabled_)
    {
        camera_.setTheta(  camera_.theta() - GLWIDGET_PI/24 );
        update();
    }
}

void GLWidget::zoomIn()
{
    if(isBusy())
      return;

    if(cameraZoomIsEnabled_)
    {
        // 3D
        double ratio = 0.8;
        camera_.setR( camera_.r() * ratio );

        // 2D
        ratio = 1/ratio;
        camera2D_.setZoom( camera2D_.zoom() * ratio );
        camera2D_.setX( mouse_Event_X_ + ratio * ( camera2D_.x() - mouse_Event_X_ ) );
        camera2D_.setY( mouse_Event_Y_ + ratio * ( camera2D_.y() - mouse_Event_Y_ ) );

        emit viewChanged(mouse_Event_X_, mouse_Event_Y_);
    }
}

void GLWidget::zoomOut()
{
    if(isBusy())
      return;

    if(cameraZoomIsEnabled_)
    {
        // 3D
        double ratio = 1/0.8;
        camera_.setR( camera_.r() * ratio );

        // 2D
        ratio = 1/ratio;
        camera2D_.setZoom( camera2D_.zoom() * ratio );
        camera2D_.setX( mouse_Event_X_ + ratio * ( camera2D_.x() - mouse_Event_X_ ) );
        camera2D_.setY( mouse_Event_Y_ + ratio * ( camera2D_.y() - mouse_Event_Y_ ) );

        emit viewChanged(mouse_Event_X_, mouse_Event_Y_);
    }
}




/*********************************************************
 *                     Events 
 */


void GLWidget::keyPressEvent(QKeyEvent *event)
{
    event->ignore();

    if(!isOnly2D_)
    {
        switch(event->key()){
        case Qt::Key_Up:
            rotateViewUp();
            break;
        case Qt::Key_Down:
            rotateViewDown();
            break;
        case Qt::Key_Right:
            rotateViewRight();
            break;
        case Qt::Key_Left:
            rotateViewLeft();
            break;
        case Qt::Key_O:
            isOrtho_ = !isOrtho_;
            update();
            break;

        default:
            event->ignore();
        }
    }
    else
        event->ignore();

}

void GLWidget::keyReleaseEvent(QKeyEvent *event)
{
    event->ignore();
}

/************************************************************
 *             Semantics of mouse events
 */


int GLWidget::decideClicAction()
{
    // by default, there is no clic action
    return GLAction::None;
}

int GLWidget::decidePMRAction()
{
    if(!isOnly2D_)
    {
    //if(mouse_AltWasDown_)
      //{
        // this is a 3D navigation function
        if(mouse_LeftButton_)
            return GLAction::Dolly;
        else if (mouse_MidButton_)
            return GLAction::Travelling;
        else if (mouse_RightButton_)
            return GLAction::Zoom;
      //}
    }
    else
    {
        if (mouse_MidButton_)
        {
            if(mouse_AltWasDown_)
                return GLAction::Zoom;
            else
                return GLAction::Travelling;
        }
    }
    
    return GLAction::None;
}

void GLWidget::MoveEvent(double /*xScene*/, double /*yScene*/)
{
    // nothing to do
}

void GLWidget::ClicEvent(int /*action*/, double /*xScene*/, double /*yScene*/)
{
    // nothing to do
}

void GLWidget::PMRPressEvent(int action, double /*xScene*/, double /*yScene*/)
{
    // nothing to do  in this case, since 3d  navigation PRM actions
    // does not perform  something specific when initialising (could
    // be possible  though, since we  know that no  geometry changes
    // will occur  in the next  drawing, we could use  display lists
    // here, for instance)


    if(action == GLAction::Dolly ||
       action == GLAction::Travelling ||
       action == GLAction::Zoom)
    {
        emit viewIsGoingToChange(mouse_Event_X_, mouse_Event_Y_);
    }

}

void GLWidget::PMRMoveEvent(int action, double /*xScene*/, double /*yScene*/)
{
    // Dolly
    if(action == GLAction::Dolly && cameraDollyIsEnabled_){
        int dx = mouse_Event_X_ - mouse_PressEvent_X_;
        int dy = mouse_Event_Y_ - mouse_PressEvent_Y_;
        
        double mouseSensibility = 2*GLWIDGET_PI/1000.0;
        
        camera_.setPhi( camera_beforeMousePress_.phi() - dx * mouseSensibility );
        camera_.setTheta( camera_beforeMousePress_.theta() + dy * mouseSensibility );

        autoCenterScene_ = false;
        emit viewIsBeingChanged(mouse_Event_X_, mouse_Event_Y_);
    }
    
    // Travelling
    else if(action == GLAction::Travelling && cameraTravellingIsEnabled_){
        double dx = mouse_Event_X_ - mouse_PressEvent_X_;
        double dy = mouse_Event_Y_ - mouse_PressEvent_Y_;
        
        if(isOnly2D_)
        {
            camera2D_.setX( camera2D_beforeMousePress_.x() + dx);
            camera2D_.setY( camera2D_beforeMousePress_.y() + dy);
        }
        else
        {
            double ratio = 2.0 * camera_.r() * tan( camera_.fovy() / 2.0) / (double) height();

            double dx_gl = ratio * dx;
            double dy_gl = ratio * dy;

            GLdouble cos_theta = cos(camera_.theta());
            GLdouble sin_theta = sin(camera_.theta());
            GLdouble cos_phi = cos(camera_.phi());
            GLdouble sin_phi = sin(camera_.phi());

            camera_.setFocus_x(
                        camera_beforeMousePress_.focus_x()
                        + dx_gl * (-cos_phi)
                        + dy_gl * (- sin_theta * sin_phi) );
            camera_.setFocus_y(
                        camera_beforeMousePress_.focus_y()
                        + 0
                        + dy_gl * cos_theta);
            camera_.setFocus_z(
                        camera_beforeMousePress_.focus_z()
                        + dx_gl * sin_phi
                        + dy_gl * (- sin_theta * cos_phi) );
        }

        autoCenterScene_ = false;
        emit viewIsBeingChanged(mouse_Event_X_, mouse_Event_Y_);
    }

      // Zoom
      else if(action == GLAction::Zoom && cameraZoomIsEnabled_)
      {
          double dr = (mouse_Event_X_ - mouse_PressEvent_X_) - (mouse_Event_Y_ - mouse_PressEvent_Y_);

          // 3D
          double ratio = pow(2.0, -dr/200.0);
          camera_.setR( camera_beforeMousePress_.r() * ratio );

          // 2D
          ratio = 1/ratio;
          camera2D_.setZoom( camera2D_beforeMousePress_.zoom() * ratio );
          camera2D_.setX( mouse_PressEvent_X_ + ratio * ( camera2D_beforeMousePress_.x() - mouse_PressEvent_X_ ) );
          camera2D_.setY( mouse_PressEvent_Y_ + ratio * ( camera2D_beforeMousePress_.y() - mouse_PressEvent_Y_ ) );


          // Hide cursor, only when start moving
          // Good idea, but DOESN'T WORK FOR TABLETS
          //if(!mouse_HideCursor_)
          //{
            //  setCursor( QCursor( Qt::BlankCursor ) );
            //  mouse_HideCursor_ = true;
          //}
          // recenter cursor
          //QCursor::setPos(mapToGlobal(QPoint(mouse_PressEvent_X_,mouse_PressEvent_Y_)));

          autoCenterScene_ = false;
          emit viewIsBeingChanged(mouse_Event_X_, mouse_Event_Y_);
      }
}

void GLWidget::PMRReleaseEvent(int action, double /*xScene*/, double /*yScene*/)
{
    // DOESN'T WORK FOR TABLETS
    //if(action == GLAction::Zoom && cameraZoomIsEnabled_)
    //{
    //    mouse_HideCursor_ = false;
    //    QCursor::setPos(mapToGlobal(QPoint(mouse_PressEvent_X_,mouse_PressEvent_Y_)));
    //    emit viewChanged();
    //}

    if(action == GLAction::Dolly ||
       action == GLAction::Travelling ||
       action == GLAction::Zoom)
    {
        emit viewChanged(mouse_Event_X_, mouse_Event_Y_);
    }
}



/*********************************************************************
 *                  Architecture of mouse events
 */

void GLWidget::mousePressEvent(QMouseEvent * event)
{
    delegateMousePress(event);
}

void GLWidget::delegateMousePress(QMouseEvent * event)
{
    emit mousePressed(this);

    // ignore other clics if a mouse button is already pressed
    if( mouse_LeftButton_ || mouse_MidButton_ || mouse_RightButton_)
        return;

    // determines which clic has been done
    mouse_LeftButton_ = false;
    mouse_MidButton_ = false;
    mouse_RightButton_ = false;
    switch(event->button()){
    case Qt::LeftButton:
        mouse_LeftButton_ = true;
        break;
    case Qt::MidButton:
        mouse_MidButton_ = true;
        break;
    case Qt::RightButton:
        mouse_RightButton_ = true;
        break;
    default:
        event->ignore();
        return;
    }

    // determines which modifiers are down
    mouse_AltWasDown_ = false;
    mouse_ControlWasDown_ = false;
    mouse_ShiftWasDown_ = false;
    if(global()->keyboardModifiers() & Qt::AltModifier)
        mouse_AltWasDown_ = true;
    if(global()->keyboardModifiers() & Qt::ControlModifier)
        mouse_ControlWasDown_ = true;
    if(global()->keyboardModifiers() & Qt::ShiftModifier)
        mouse_ShiftWasDown_ = true;

    // save data when the mousePressEvent occured
    mouse_PressEvent_X_ = event->x();
    mouse_PressEvent_Y_ = event->y();
    camera_beforeMousePress_ = camera_;
    camera2D_beforeMousePress_ = camera2D_;

    // convert to scene coordinates
    if(isOnly2D_)
    {
        Eigen::Vector3d p = camera2D_.viewMatrixInverse() * Eigen::Vector3d(mouse_PressEvent_X_, mouse_PressEvent_Y_, 0);
        mouse_PressEvent_XScene_ = p[0];
        mouse_PressEvent_YScene_ = p[1];
    }

    // this is also the current mouse coordinates
    mouse_Event_X_ = mouse_PressEvent_X_;
    mouse_Event_Y_ = mouse_PressEvent_Y_;
    mouse_Event_XScene_ = mouse_PressEvent_XScene_;
    mouse_Event_YScene_ = mouse_PressEvent_YScene_;


    // decides what actions are possible
    mouse_ClicAction_ = decideClicAction();
    mouse_PMRAction_ = decidePMRAction();

    // if we know it's a PMR action, generate the press event now
    if(!mouse_ClicAction_ && mouse_PMRAction_)
        PMRPressEvent(mouse_PMRAction_,
                      mouse_PressEvent_XScene_, mouse_PressEvent_YScene_);
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    delegateMouseMove(event);
}

#include <QElapsedTimer>

void GLWidget::delegateMouseMove(QMouseEvent *event)
{
    emit mouseMoved(this);

    // get window coordinates of event
    mouse_Event_X_ = event->x();
    mouse_Event_Y_ = event->y();

    // convert to scene coordinates
    if(isOnly2D_)
    {
        Eigen::Vector3d p = camera2D_.viewMatrixInverse() * Eigen::Vector3d(mouse_Event_X_, mouse_Event_Y_, 0);
        mouse_Event_XScene_ = p[0];
        mouse_Event_YScene_ = p[1];
    }

    // if there  is no mouse  button down, it  is a void  move event
    if(!(mouse_LeftButton_ || mouse_MidButton_ || mouse_RightButton_))
    {
        qint64 idleTime = mouse_timerIdleTime_.elapsed();
        if(idleTime>1)
        {
            MoveEvent(mouse_Event_XScene_, mouse_Event_YScene_);
        }
        mouse_timerIdleTime_.start();
        return;
    }
    
      // if there is no possible PMR actions, ignore this
      if(mouse_PMRAction_ == GLAction::None)
        return;

      // determines if the action should become a PMR action
      if(mouse_ClicAction_)
      {
        if(abs(mouse_PressEvent_X_ - mouse_Event_X_) > MIN_SIZE_DRAWING ||
           abs(mouse_PressEvent_Y_ - mouse_Event_Y_) > MIN_SIZE_DRAWING)
        {
            PMRPressEvent(mouse_PMRAction_,
                      mouse_PressEvent_XScene_, mouse_PressEvent_YScene_);
            mouse_ClicAction_ = GLAction::None;
        }
    }

    // calls the move event
    if(!mouse_ClicAction_)
    {
        qint64 idleTime = mouse_timerIdleTime_.elapsed();
        if(idleTime>1)
        {
            PMRMoveEvent(mouse_PMRAction_, mouse_Event_XScene_, mouse_Event_YScene_);
        }
        mouse_timerIdleTime_.start();
    }
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    delegateMouseRelease(event);
}
void GLWidget::delegateMouseRelease(QMouseEvent *event)
{
    emit mouseReleased(this);

      // if there is no mouse button down, just ignore this
      if(!(mouse_LeftButton_ || mouse_MidButton_ || mouse_RightButton_))
        return;

      // get window coordinates of event
      mouse_Event_X_ = event->x();
      mouse_Event_Y_ = event->y();

      // convert to scene coordinates
      if(isOnly2D_)
      {
          Eigen::Vector3d p = camera2D_.viewMatrixInverse() * Eigen::Vector3d(mouse_Event_X_, mouse_Event_Y_, 0);
          mouse_Event_XScene_ = p[0];
          mouse_Event_YScene_ = p[1];
      }

      // set the mouse button state to false, only if it's the 
      // corresponding button 
      switch(event->button()){
      case Qt::LeftButton:
        if(!mouse_LeftButton_)
          return;
        mouse_LeftButton_ = false;
        break;
      case Qt::MidButton:
        if(!mouse_MidButton_)
          return;
        mouse_MidButton_ = false;
        break;
      case Qt::RightButton:
        if(!mouse_RightButton_)
          return;
        mouse_RightButton_ = false;
        break;
      default:
        event->ignore();
        return;
      }

      // if there is no possible actions, ignore this too
      if(!mouse_ClicAction_ && !mouse_PMRAction_)
        return;

      // Perform the corresponding actions
    if(mouse_ClicAction_)
        ClicEvent(mouse_ClicAction_, mouse_Event_XScene_, mouse_Event_YScene_);
    else
        PMRReleaseEvent(mouse_PMRAction_, mouse_Event_XScene_, mouse_Event_YScene_);

    // Inform that the GLView is not busy anymore
    mouse_ClicAction_ = GLAction::None;
    mouse_PMRAction_ = GLAction::None;
}

void GLWidget::tabletEvent(QTabletEvent * event)
{
    // Set pressure
    mouse_tabletPressure_ = event->pressure();

    // Delegate
    if(event->type() == QEvent::TabletPress)
    {
        mouse_isTablet_ = true;
    }
    else if(event->type() == QEvent::TabletMove)
    {
    }
    else if(event->type() == QEvent::TabletRelease)
    {
        mouse_isTablet_ = false;
    }
    else
    {
    }

    // Ignore event, so Qt generates a mouse event.
    event->ignore();
}

void GLWidget::wheelEvent(QWheelEvent *event)
{
    
      if(isBusy())
        return;

    if(cameraZoomIsEnabled_)
    {
        // Ratio to apply
        double ratio = pow( 0.8f, (double) event->delta() / (double) 120.0f);

        // Clamp to avoid getting too far out
        double x = camera2D_.zoom() / ratio;
        if(x<0.01)
            x = 0.01;
        if(x>1e5)
            x = 1e5;
        ratio = camera2D_.zoom() / x;

        // 3D
        camera_.setR( camera_.r() * ratio );

        // 2D
        ratio = 1/ratio;
        camera2D_.setZoom( camera2D_.zoom() * ratio );
        camera2D_.setX( mouse_Event_X_ + ratio * ( camera2D_.x() - mouse_Event_X_ ) );
        camera2D_.setY( mouse_Event_Y_ + ratio * ( camera2D_.y() - mouse_Event_Y_ ) );

        autoCenterScene_ = false;
        emit viewChanged(mouse_Event_X_, mouse_Event_Y_);
    }
}

void GLWidget::enterEvent(QEvent * /*event*/)
{
    emit mouseEntered(this);
}

void GLWidget::leaveEvent(QEvent * /*event*/)
{
    emit mouseLeft(this);
}








/*********************************************************************
 *          Setting up of rendering and viewing options 
 */

void GLWidget::initializeGL()
{
    qInfo() << "Initializing OpenGL, using the following format:";
    qInfo() << format();

    // Access OpenGL 2.1 functions
    if (!gl_) {
        gl_ = context()->versionFunctions<OpenGLFunctions>();
        if (!gl_) {
            qFatal("Failed to access OpenGL " VPAINT_OPENGL_VERSION " functions.");
        }
    }

    // Query extensions
    bool queryExtensions = false;
    if (queryExtensions) {
        QList<QByteArray> extensions = context()->extensions().toList();
        qDebug() << "Supported extensions (" << extensions.count() << ")";
        foreach (const QByteArray &extension, extensions)
            qDebug() << "    " << extension;
    }

    // Access GL_ARB_framebuffer_object extension
    if (!gl_fbo_) {
        if (!context()->hasExtension(QByteArrayLiteral("GL_ARB_framebuffer_object"))) {
            qFatal("GL_ARB_framebuffer_object is not supported");
        }
        gl_fbo_.reset(new QOpenGLExtension_ARB_framebuffer_object());
        gl_fbo_->initializeOpenGLFunctions();
    }

    // Depth test
    if(isOnly2D_)
        glDisable(GL_DEPTH_TEST);
    else
        glEnable(GL_DEPTH_TEST);

    // Shading mode
    glShadeModel(GL_SMOOTH);

    // Alpha blending
    glEnable(GL_BLEND);
    gl_->glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
                             GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    
    // Line Antialiasing
    //glEnable( GL_LINE_SMOOTH );
    //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
}

void GLWidget::resizeGL(int width, int height)
{
    Q_UNUSED(width);
    Q_UNUSED(height);

    emit viewResized();
}



void GLWidget::setLighting()
{
    if(isOnly2D_)
    {
        glDisable(GL_LIGHTING);
        return;
    }
    
      glEnable(GL_LIGHTING);


    // ---- Simple lighting ----
    
      // Ambient light
    
      GLfloat global_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
      glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
      glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
      glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);

    // Light of the camera
    
      GLdouble cos_theta = cos(camera_.theta());
    GLdouble sin_theta = sin(camera_.theta());
    GLdouble cos_phi = cos(camera_.phi());
    GLdouble sin_phi = sin(camera_.phi());

    GLWidget_Light cameraLight(
        camera_.focus_x() + camera_.r() * cos_theta * sin_phi,
        camera_.focus_y() + camera_.r() * sin_theta,
        camera_.focus_z() + camera_.r()  * cos_theta * cos_phi,
        0.4f, 0.4f, 0.4f, QStringLiteral("Camera light"));
    cameraLight.lookAt(
         camera_.focus_x(),
         camera_.focus_y(),
         camera_.focus_z());

    lights_.append(cameraLight);
    
    // Constant lights
      int n = 0;
      for(QList<GLWidget_Light>::iterator it = lights_.begin();
      it != lights_.end();
      ++it)
      {
        if(n>7)
          break;
        
        glEnable(GL_LIGHT_[n]);
        
        float ambient[] = {
          it->ambient_r,
          it->ambient_g,
          it->ambient_b,
          it->ambient_a
        };
        float diffuse[] = {
          it->diffuse_r,
          it->diffuse_g,
          it->diffuse_b,
          it->diffuse_a
        };
        float specular[] = {
          it->specular_r,
          it->specular_g,
          it->specular_b,
          it->specular_a
        };
        float position[] = {
          it->position_x,
          it->position_y,
          it->position_z,
          it->position_w
        };
        float spotDirection[] = {
          it->spotDirection_x,
          it->spotDirection_y,
          it->spotDirection_z,
        };
        
        glLightfv(GL_LIGHT_[n], GL_AMBIENT, ambient);
        glLightfv(GL_LIGHT_[n], GL_DIFFUSE, diffuse);
        glLightfv(GL_LIGHT_[n], GL_SPECULAR, specular);
        glLightfv(GL_LIGHT_[n], GL_POSITION, position);
        glLightf(GL_LIGHT_[n], GL_CONSTANT_ATTENUATION,
              it->constantAttenuation);
        glLightf(GL_LIGHT_[n], GL_LINEAR_ATTENUATION,
              it->linearAttenuation);
        glLightf(GL_LIGHT_[n], GL_QUADRATIC_ATTENUATION,
              it->quadraticAttenuation);
        
        glLightfv(GL_LIGHT_[n], GL_SPOT_DIRECTION, spotDirection);
        glLightf(GL_LIGHT_[n], GL_SPOT_CUTOFF, it->spotCutoff);
        glLightf(GL_LIGHT_[n], GL_SPOT_EXPONENT, it->spotExponent);
      
        n++;
      }
      for(int i=n; i<8; i++)
        glDisable(GL_LIGHT_[i]);

    lights_.removeLast();

}

// GLU was removed from Qt in version 4.8  
// TODO: remove this dependancy and code it myself 
#ifdef Q_OS_MAC  
# include <OpenGL/glu.h>  
#else  
# include <GL/glu.h>  
#endif  

void GLWidget::setCameraPositionAndOrientation()
{
    if(isOnly2D_)
    {
        // Set projection such that Camera/View coords = Device/Window coords
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, width(), height(), 0, 0, 1);

        // Set view
        glMatrixMode (GL_MODELVIEW);
        glLoadMatrixd(camera2D_.viewMatrixData());
    }
    else
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity ();
        if(isOrtho_)
        {
            double h = 2 * camera_.r() * tan(camera_.fovy()/2.0);
            double ratio = h / height();
            double w = width() * ratio;

            glOrtho (-0.5*w, 0.5*w, -0.5*h, 0.5*h, 0, 100);
        
        }
        else
        {
            
            //TODO: recode this function without glu
              gluPerspective(
              camera_.fovy() * 180 / GLWIDGET_PI,
              (double) width() / (double) height(),
              0.1, 100);
            
        }
        
        Eigen::Vector3d pos = camera_.position();
        Eigen::Vector3d focus = camera_.focusPoint();
        Eigen::Vector3d up = camera_.upDirection();

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        //  TODO: recode this function without glu
        gluLookAt(pos[0], pos[1], pos[2],
                focus[0], focus[1], focus[2],
                up[0], up[1], up[2]);
        
    }    
}



void GLWidget::setMaterial(const GLWidget_Material & m)
{
      float ambientFront[] = { 
        m.ambientFront_r,
        m.ambientFront_g,
        m.ambientFront_b,
        m.ambientFront_a 
      };
      glMaterialfv(GL_FRONT, GL_AMBIENT, ambientFront);
        
      float diffuseFront[] = { 
        m.diffuseFront_r,
        m.diffuseFront_g,
        m.diffuseFront_b,
        m.diffuseFront_a 
      };
      glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseFront);
          
      float specularFront[] = { 
        m.specularFront_r,
        m.specularFront_g,
        m.specularFront_b,
        m.specularFront_a 
      };
      glMaterialfv(GL_FRONT, GL_SPECULAR, specularFront);
                    
      float emissionFront[] = { 
        m.emissionFront_r,
        m.emissionFront_g,
        m.emissionFront_b,
        m.emissionFront_a 
      };
      glMaterialfv(GL_FRONT, GL_EMISSION, emissionFront);
                    
      glMateriali(GL_FRONT, GL_SHININESS, m.shininessFront);
        
      float ambientBack[] = { 
        m.ambientBack_r,
        m.ambientBack_g,
        m.ambientBack_b,
        m.ambientBack_a 
      };
      glMaterialfv(GL_BACK, GL_AMBIENT, ambientBack);
        
      float diffuseBack[] = { 
        m.diffuseBack_r,
        m.diffuseBack_g,
        m.diffuseBack_b,
        m.diffuseBack_a 
      };
      glMaterialfv(GL_BACK, GL_DIFFUSE, diffuseBack);
          
      float specularBack[] = { 
        m.specularBack_r,
        m.specularBack_g,
        m.specularBack_b,
        m.specularBack_a 
      };
      glMaterialfv(GL_BACK, GL_SPECULAR, specularBack);
                    
      float emissionBack[] = { 
        m.emissionBack_r,
        m.emissionBack_g,
        m.emissionBack_b,
        m.emissionBack_a 
      };
      glMaterialfv(GL_BACK, GL_EMISSION, emissionBack);
                    
      glMateriali(GL_BACK, GL_SHININESS, m.shininessBack);
}












/*********************************************************************
 *                    General drawing functions 
 */

GLWidget * GLWidget::currentGLWidget_ = 0;
void GLWidget::paintGL()
{
    // ---- Drawing all opaques objects (here, mainly lines) ----
    glDepthMask(GL_TRUE);
    
    // Clear the window and buffers
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Initialize the view and lighting
    setCameraPositionAndOrientation();
    setLighting();

    // Set material (this is required for the 3D vew)
    // XXX this shouldn't be done here
    setMaterial(material_);
    glEnable(GL_COLOR_MATERIAL);

    // Draw scene
    currentGLWidget_ = this;
    drawScene();
    currentGLWidget_ = 0;
}

void GLWidget::drawScene()
{
}




/*********************************************************************
 *                    Individual Drawings
 */


// the 3-axis box
void GLWidget::drawGrid()
{
    glDisable(GL_LIGHTING);

      glLineWidth(1.0);
    glBegin(GL_LINES);    
      glColor3d(0.5, 0.5, 0.5);
      for(int i=-10; i<= 10; i++){
        if(i!=0){
          // if(xOy)
          glVertex3d(i/10.0, -1, -1);
          glVertex3d(i/10.0, 1, -1);
          glVertex3d(-1, i/10.0, -1);
          glVertex3d(1, i/10.0, -1);
          
          // if(xOz)
          glVertex3d(i/10.0, -1, -1);
          glVertex3d(i/10.0, -1, 1);
          glVertex3d(-1, -1, i/10.0);
          glVertex3d(1, -1, i/10.0);
          
          // if(yOz)
          glVertex3d(-1, i/10.0, -1);
          glVertex3d(-1, i/10.0, 1);
          glVertex3d(-1, -1, i/10.0);
          glVertex3d(-1, 1, i/10.0);
        }
      }
      glEnd();
    
      glLineWidth(2.0);
    glBegin(GL_LINES);    
      
      // if(xOy)
      glColor3d(0, 0, 1);
      glVertex3d(0, -1, -1);
      glVertex3d(0, 1, -1);
      glVertex3d(-1, 0, -1);
      glVertex3d(1, 0, -1);
      
      // if(xOz)
      glColor3d(0, 0.9, 0);
      glVertex3d(0, -1, -1);
      glVertex3d(0, -1, 1);
      glVertex3d(-1, -1, 0);
      glVertex3d(1, -1, 0);
      
      // if(yOz)
      glColor3d(1, 0, 0);
      glVertex3d(-1, 0, -1);
      glVertex3d(-1, 0, 1);
      glVertex3d(-1, -1, 0);
      glVertex3d(-1, 1, 0);
      
      glEnd();
      glLineWidth(1.0);
      glEnable(GL_LIGHTING);
}

