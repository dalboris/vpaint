// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef SCENE_OBJECT_EXAMPLE_H
#define SCENE_OBJECT_EXAMPLE_H

#include "OpenGL.h"
#include "SceneObject.h"
#include <QSet>

class SceneObject_Example : public SceneObject
{
public:
    SceneObject_Example() : isHighlighted_(false) {}
    
    void draw(Time time, ViewSettings & viewSettings)
        {
            if(selectedFrames_.contains(time.frame))
            {
                if(isHighlighted_ && time.frame == highlightedFrame_)
                    glColor3d(1,0.5,0.5);
                else
                    glColor3d(1,0,0);
            }
            else
            {
                if(isHighlighted_ && time.frame == highlightedFrame_)
                    glColor3d(0.5,0.5,1);
                else
                    glColor3d(0,0,1);
            }
            
            rawDraw(time);
        }
    void drawPick(Time time, ViewSettings & /*viewSettings*/)
        {
            Picking::glColor(42);
            rawDraw(time);
        }

    void rawDraw(Time time, ViewSettings & /*viewSettings*/)
        {
            double t = time.time;
            
            glPointSize(10);
            glLineWidth(2);
            glBegin(GL_LINE_STRIP);
            glVertex2d(100+100*t, 100);
            glVertex2d(200, 250);
            glEnd();
            glBegin(GL_POINTS);
            glVertex2d(100+100*t, 100);
            glVertex2d(200, 250);
            glEnd();
        }

    void setHoveredObject(Time time, int /*id*/) 
        {
            if(!isHighlighted_)
            {
                isHighlighted_=true;
                highlightedFrame_ = time.frame;
                emit changed();
            }
        }
    void setNoHoveredObject()
        {
            if(isHighlighted_)
            {
                isHighlighted_=false;
                emit changed();
            }
        } 
    void select(Time time, int id)
        {
            selectedFrames_.insert(time.frame);
            emit changed();
        }
    void deselect(Time time, int id)
        {
            deselectAll(time);
        }
    void toggle(Time time, int id)
        {
            if(selectedFrames_.contains(time.frame))
                deselect(time, id);
            else
                select(time, id);
        }
    void deselectAll(Time time)
        {
            selectedFrames_.remove(time.frame);
            emit changed();
        }
    void deselectAll()
        {
            selectedFrames_.clear();
            emit changed();
        }

private:
    bool isHighlighted_;
    int highlightedFrame_;

    QSet<int> selectedFrames_;
    
};

#endif
