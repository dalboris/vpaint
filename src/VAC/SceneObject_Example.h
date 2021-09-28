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

#ifndef SCENE_OBJECT_EXAMPLE_H
#define SCENE_OBJECT_EXAMPLE_H

#include "OpenGL.h"
#include "SceneObject.h"
#include <QSet>
#include "vpaint_global.h"

class Q_VPAINT_EXPORT SceneObject_Example : public SceneObject
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
