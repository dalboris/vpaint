// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "View3D.h"
#include "Scene.h"
#include "Timeline.h"
#include <QtDebug>
#include "OpenGL.h"
#include "Global.h"
#include "View.h"

#include "VectorAnimationComplex/VAC.h"

// define mouse actions

#define DRAW_ACTION 10
#define SELECT_ACTION 20
#define ADDSELECT_ACTION 21
#define DESELECT_ACTION 22
#define TOGGLESELECT_ACTION 23
#define DESELECTALL_ACTION 24

View3D::View3D(Scene *scene, QWidget *parent) :
    GLWidget(parent, false), // Difference from View here
    scene_(scene),
    displayedTimes_(),
    pickingImg_(0),
    //frame_(0),
    vac_(0)
{

    viewSettingsWidget_ = new View3DSettingsWidget(viewSettings_);
    viewSettingsWidget_->setParent(this, Qt::Window);
    connect(viewSettingsWidget_, SIGNAL(changed()), this, SLOT(update()));

    cameraTravellingIsEnabled_ = true;
    drawingIsEnable_ = false;

    // behave as a separate window
    this->setWindowFlags(Qt::Window);
    resize(600,600);
    setWindowTitle("3D View [Beta]");

    // Redraw when moving the camera
    //connect(this, SIGNAL(viewIsGoingToChange(int, int)), this, SLOT(updatePicking()));
    ////connect(this, SIGNAL(viewIsGoingToChange(int, int)), this, SLOT(updateHighlightedObject(int, int)));
    //connect(this, SIGNAL(viewIsGoingToChange(int, int)), this, SLOT(update()));

    ////connect(this, SIGNAL(viewIsBeingChanged(int, int)), this, SLOT(updatePicking()));
    ////connect(this, SIGNAL(viewIsBeingChanged(int, int)), this, SLOT(updateHighlightedObject(int, int)));
    connect(this, SIGNAL(viewIsBeingChanged(int, int)), this, SLOT(update()));

    //connect(this, SIGNAL(viewChanged(int, int)), this, SLOT(updatePicking()));
    //connect(this, SIGNAL(viewChanged(int, int)), this, SLOT(updateHighlightedObject(int, int)));
    connect(this, SIGNAL(viewChanged(int, int)), this, SLOT(update()));

    //connect(global(),SIGNAL(keyboardModifiersChanged()),this,SLOT(handleNewKeyboardModifiers()));

    connect(global()->timeline(), SIGNAL(playingWindowChanged()), this, SLOT(update()));
}

View3D::~View3D()
{
    deletePicking();
}

void View3D::openViewSettings()
{
    viewSettingsWidget_->show();
}

void View3D::closeViewSettings()
{
    viewSettingsWidget_->hide();
}

View3DSettingsWidget * View3D::view3DSettingsWidget() const
{
    return viewSettingsWidget_;
}

void View3D::closeEvent(QCloseEvent * event)
{
    emit closed();
    event->accept();
}

void View3D::keyPressEvent(QKeyEvent *event)
{
    GLWidget::keyPressEvent(event);

    // Nothing changed  in the scene, but keyboard  state can affect
    // the display settings, hence should redraw the scene. Pass the
    // event to multiview to do this
    if(!event->isAccepted())
        event->ignore();
}

      
View3D::MouseEvent View3D::mouseEvent() const
{
    MouseEvent me;
    me.x = mouse_PressEvent_X_;
    me.y = mouse_PressEvent_Y_;
    me.left = mouse_LeftButton_;
    me.mid = mouse_MidButton_;
    me.right = mouse_RightButton_;
    me.alt = mouse_AltWasDown_;
    me.control = mouse_ControlWasDown_;
    me.shift = mouse_ShiftWasDown_;
    return me;
}

void View3D::update()
{
    updateGL();
    // at first, I've  put updatePicking here, but it  has an issue:
    // when  the  window  is  resized, updateGL()  is  automatically
    // called, but then not updatePicking...
    //
    // Hence, it  is now called  inside drawScene, even  though it's
    // kinda weird
}


void View3D::MoveEvent(double x, double y)
{
    bool hasChanged = updateHighlightedObject(x, y);
    if(hasChanged)
    {
        if(highlightedObject_.isNull())
            scene_->setNoHoveredObject();
        else
            scene_->setHoveredObject(
                //Timeline::time(highlightedObject_.time()),
                Time(), // ignored by VAC anyway...
                highlightedObject_.index(),
                highlightedObject_.id());
    }
}

int View3D::decideClicAction()
{
    if(mouse_LeftButton_)
    {
        if(!mouse_AltWasDown_ &&
           !mouse_ControlWasDown_ &&
           !mouse_ShiftWasDown_)
        {
            if(highlightedObject_.isNull())
                return DESELECTALL_ACTION;
            else
                return SELECT_ACTION;
        }
        if(!mouse_AltWasDown_ &&
           !mouse_ControlWasDown_ &&
           mouse_ShiftWasDown_)
        {
            return ADDSELECT_ACTION;
        }
        if(mouse_AltWasDown_ &&
           !mouse_ControlWasDown_ &&
           !mouse_ShiftWasDown_)
        {
            return DESELECT_ACTION;
        }
        if(mouse_AltWasDown_ &&
           !mouse_ControlWasDown_ &&
           mouse_ShiftWasDown_)
        {
            return TOGGLESELECT_ACTION;
        }
    }    
    return GLWidget::decideClicAction();
}

int View3D::decidePMRAction()
{
    return GLWidget::decidePMRAction();
}

void View3D::ClicEvent(int action, double x, double y)
{
    if(action==SELECT_ACTION)
    {
        if(!highlightedObject_.isNull())
        {
            scene_->deselectAll(); // deselect       at      all
                             // times. different from 2D behaviour
            scene_->select(Time() /*Timeline::time(highlightedObject_.time())*/,
                       highlightedObject_.index(),
                       highlightedObject_.id());
        }
    }
    else if(action==DESELECTALL_ACTION)
    {
        // same here: deselect at all times
        // different from 2D version
        scene_->deselectAll();
        
    }
    else if(action==ADDSELECT_ACTION)
    {
        if(!highlightedObject_.isNull())
        {
            scene_->select(Time()/*Timeline::time(highlightedObject_.time())*/,
                       highlightedObject_.index(),
                       highlightedObject_.id());
        }
    }
    else if(action==DESELECT_ACTION)
    {
        if(!highlightedObject_.isNull())
        {
            scene_->deselect(Time()/*Timeline::time(highlightedObject_.time())*/,
                         highlightedObject_.index(),
                         highlightedObject_.id());
        }
    }
    else if(action==TOGGLESELECT_ACTION)
    {
        if(!highlightedObject_.isNull())
        {
            scene_->toggle(Time()/*Timeline::time(highlightedObject_.time())*/,
                       highlightedObject_.index(),
                       highlightedObject_.id());
        }
    }
    GLWidget::ClicEvent(action, x, y);
}



void View3D::PMRPressEvent(int action, double x, double y)
{
    GLWidget::PMRPressEvent(action, x, y);
}

void View3D::PMRMoveEvent(int action, double x, double y)
{
    GLWidget::PMRMoveEvent(action, x, y);
}

void View3D::PMRReleaseEvent(int action, double x, double y)
{
    GLWidget::PMRReleaseEvent(action, x, y);
}

/***********************************************************
 *              DRAWING
 */

namespace
{
/*
void drawSphere(double r, int lats, int longs)
{
    int i, j;
    for(i = 0; i <= lats; i++)
    {
        double lat0 = M_PI * (-0.5 + (double) (i - 1) / lats);
        double z0  = sin(lat0);
        double zr0 =  cos(lat0);

        double lat1 = M_PI * (-0.5 + (double) i / lats);
        double z1 = sin(lat1);
        double zr1 = cos(lat1);

        glBegin(GL_QUAD_STRIP);
        for(j = 0; j <= longs; j++)
        {
            double lng = 2 * M_PI * (double) (j - 1) / longs;
            double x = cos(lng);
            double y = sin(lng);

            glNormal3f(x * zr0, y * zr0, z0);
            glVertex3f(r * x * zr0, r * y * zr0, r * z0);
            glNormal3f(x * zr1, y * zr1, z1);
            glVertex3f(r * x * zr1, r * y * zr1, r * z1);
        }
        glEnd();
    }
}
*/
}


void View3D::drawScene()
{
    if(!viewSettings_.freezeSpaceRect())
    {
        View * activeView = global()->activeView();
        if(activeView)
        {
            viewSettings_.setXSceneMin(activeView->xSceneMin());
            viewSettings_.setXSceneMax(activeView->xSceneMax());
            viewSettings_.setYSceneMin(activeView->ySceneMin());
            viewSettings_.setYSceneMax(activeView->ySceneMax());
        }
    }

    if(scene_->vectorAnimationComplex())
    {
        glEnable(GL_NORMALIZE);
        double s = viewSettings_.spaceScale();
        double invS = 1.0 / s;
        glScaled(s,s,s);
        if(viewSettings_.cameraFollowActiveTime())
        {
            glTranslated(0,0,-viewSettings_.zFromT(global()->activeTime()));
        }
        scene_->vectorAnimationComplex()->draw3D(viewSettings_);
        if(viewSettings_.cameraFollowActiveTime())
        {
            glTranslated(0,0,viewSettings_.zFromT(global()->activeTime()));
        }
        glScaled(invS,invS,invS);
    }
}



/***********************************************************
 *              PICKING
 */

void View3D::drawPick3D()
{/*
    foreach(int i, displayedTimes_)
    {
    Picking::setTime(i);*/
    //Picking::setTime(0);
    if(scene_->vectorAnimationComplex())
    {
        scene_->vectorAnimationComplex()->drawPick3D(viewSettings_);
    }
}

bool View3D::updateHighlightedObject(int x, int y)
{
    if(!pickingImg_)
        return false; // otherwise the scene will keep updating

    Picking::Object old = highlightedObject_;
    if(x<0 || (uint)x>=WINDOW_SIZE_X_ || y<0 || (uint)y>=WINDOW_SIZE_Y_)
    {
        highlightedObject_ = Picking::Object();
    }
    else
    {
        highlightedObject_ = getCloserObject(x, y);
    }
    return !(highlightedObject_ == old);
}

uchar * View3D::pickingImg(int x, int y)
{
    int k = 4*( (WINDOW_SIZE_Y_ - y - 1)*WINDOW_SIZE_X_ + x);
    return &pickingImg_[k];
}

    
Picking::Object View3D::getCloserObject(int x, int y)
{
    Picking::Object noObject;
    
    if(!pickingImg_)
        return noObject; 

    int leftBorderDist = x;
    int rightBorderDist = WINDOW_SIZE_X_-1-x;
    int topBorderDist = y;
    int bottomBorderDist = WINDOW_SIZE_Y_-1-y;

    int borderDist = qMin(qMin(leftBorderDist, rightBorderDist), 
                    qMin(topBorderDist, bottomBorderDist));

    if(borderDist<0)
        return noObject;

    int D = 10;
    if(borderDist < D)
        D = borderDist;
    
    for(int d=0; d<=D; d++)
    {
        if(d==0)
        {
            uchar * p = pickingImg(x,y);
            uchar r=p[0], g=p[1], b=p[2];
            if(r!=255 || g!=255 || b!=255)
                return Picking::objectFromRGB(r,g,b);
        }
        else
        {
            // top row
            for(int varX=x-d; varX<=x+d; varX++)
            {
                uchar * p = pickingImg(varX,y-d);
                uchar r=p[0], g=p[1], b=p[2];
                if(r!=255 || g!=255 || b!=255)
                    return Picking::objectFromRGB(r,g,b);
            }
            // bottom row
            for(int varX=x-d; varX<=x+d; varX++)
            {
                uchar * p = pickingImg(varX,y+d);
                uchar r=p[0], g=p[1], b=p[2];
                if(r!=255 || g!=255 || b!=255)
                    return Picking::objectFromRGB(r,g,b);
            }
            // left column
            for(int varY=y-d; varY<=y+d; varY++)
            {
                uchar * p = pickingImg(x-d,varY);
                uchar r=p[0], g=p[1], b=p[2];
                if(r!=255 || g!=255 || b!=255)
                    return Picking::objectFromRGB(r,g,b);
            }
            // right column
            for(int varY=y-d; varY<=y+d; varY++)
            {
                uchar * p = pickingImg(x+d,varY);
                uchar r=p[0], g=p[1], b=p[2];
                if(r!=255 || g!=255 || b!=255)
                    return Picking::objectFromRGB(r,g,b);
            }
            
        }
    }

    // no object within the range
    return noObject;
}

void View3D::deletePicking()
{
    if(pickingImg_)
    {
        glDeleteFramebuffers(1, &fboId_);
        glDeleteRenderbuffers(1, &rboId_);
        glDeleteTextures(1, &textureId_);
        highlightedObject_ = Picking::Object();
        delete[] pickingImg_;
        pickingImg_ = 0;
        WINDOW_SIZE_X_ = 0;
        WINDOW_SIZE_Y_ = 0;
    }
}

void View3D::newPicking()
{
    //  code adapted from http://www.songho.ca/opengl/gl_fbo.html

    // create a texture object
    glGenTextures(1, &textureId_);
    glBindTexture(GL_TEXTURE_2D, textureId_);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, WINDOW_SIZE_X_, WINDOW_SIZE_Y_, 0,
             GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // create a renderbuffer object to store depth info
    glGenRenderbuffers(1, &rboId_);
    glBindRenderbuffer(GL_RENDERBUFFER, rboId_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
                    WINDOW_SIZE_X_, WINDOW_SIZE_Y_);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // create a framebuffer object
    glGenFramebuffers(1, &fboId_);
    glBindFramebuffer(GL_FRAMEBUFFER, fboId_);
    
    // attach the texture to FBO color attachment point
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                     GL_TEXTURE_2D, textureId_, 0);

    // attach the renderbuffer to depth attachment point
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                      GL_RENDERBUFFER, rboId_);

    // check FBO status
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
    {
        qDebug() << "ERROR void View3D::newPicking()"
               << "FBO status != GL_FRAMEBUFFER_COMPLETE";
        return;
    }

    // switch back to window-system-provided framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // allocate memory for picking
    pickingImg_ = new uchar[4 * WINDOW_SIZE_X_ * WINDOW_SIZE_Y_];
}


void View3D::updatePicking()
{
    // get the viewport size, allocate memory if necessary
    GLint m_viewport[4];
    glGetIntegerv( GL_VIEWPORT, m_viewport );
    if( !(m_viewport[2]>0) || !(m_viewport[3]>0))
    {
        deletePicking();
        return;
    }
    else if(
        pickingImg_
        && (WINDOW_SIZE_X_ == (uint)m_viewport[2])
        && (WINDOW_SIZE_Y_ == (uint)m_viewport[3]))
    {
        // necessary objects already created: do nothing
    }
    else
    {
        deletePicking();
        WINDOW_SIZE_X_ = m_viewport[2];
        WINDOW_SIZE_Y_ = m_viewport[3];
        newPicking();
    }
    
    // set rendering destination to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, fboId_);

    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw the picking
    drawPick3D();
    
    // unbind FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // extract the texture info from GPU to RAM
    glBindTexture(GL_TEXTURE_2D, textureId_); 
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pickingImg_);
    glBindTexture(GL_TEXTURE_2D, 0);
}
