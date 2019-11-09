// Copyright (C) 2012-2019 The VPaint Developers
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

#ifndef VIEW_3D_H
#define VIEW_3D_H

#include <cmath>
#include <iostream>
#include "Picking.h"
#include "GLWidget.h"
#include "GeometryUtils.h"
#include <QList>
#include <QPointF>
#include <QPoint>

#include "View3DSettings.h"

// pre-declarations

class Scene;
namespace VectorAnimationComplex { class VAC; }
class Background;
class BackgroundRenderer;

class View3D: public GLWidget
{
    Q_OBJECT
    
public:
    View3D(Scene *scene, QWidget *parent);
    virtual ~View3D();

    // Returns a non-owning pointer to the View3DSettings owned
    // by this View3D
    View3DSettings * settings();

    virtual void keyPressEvent(QKeyEvent *event);

    virtual int decideClicAction();
    virtual int decidePMRAction();
    
    virtual void MoveEvent(double x, double y);
    virtual void ClicEvent(int action, double x, double y);
    virtual void PMRPressEvent(int action, double x, double y);
    virtual void PMRMoveEvent(int action, double x, double y);
    virtual void PMRReleaseEvent(int action, double x, double y);

    virtual void drawScene();

    void setDisplayedTimes(const QList<int> & times, bool shouldUpdate = true)
        { displayedTimes_ = times;
            if(shouldUpdate) update();};

    // picking
    void deletePicking();
    Picking::Object getCloserObject(int x, int y);
    bool updateHighlightedObject(int x, int y);

    // Time info
    int activeFrame() const;
    Time activeTime() const;

public slots:
    void updatePicking();

protected slots:
    void closeEvent(QCloseEvent * event);

signals:
    void allViewsNeedToUpdate();
    void allViewsNeedToUpdatePicking();
    void closed();

private:
    
struct MouseEvent 
{
    // Mouse position
    int x, y;
    // Mouse buttons pressed: at most one of these is true
    bool left, mid, right;
    // Modifiers keys: any number of these can be true
    bool alt, control, shift;
};

    Scene *scene_;
    QList<int> displayedTimes_;
    
    // Mouse event related members
    MouseEvent mouseEvent() const;
    QPoint lastMousePos_;
    bool drawingIsEnable_;


    // picking
    void newPicking();
    void drawPick3D();
    uchar * pickingImg(int x, int y);
    GLuint WINDOW_SIZE_X_;
    GLuint WINDOW_SIZE_Y_;
    GLuint textureId_;
    GLuint rboId_;
    GLuint fboId_;
    uchar *pickingImg_;
    Picking::Object highlightedObject_;

    // Implementation details: Drawing Stroke
    //int frame_;
    VectorAnimationComplex::VAC * vac_;

    // View Settings
    View3DSettings viewSettings_;

    // Draw canvas
    void drawCanvas_();

    // Draw background
    void drawBackground_(Background * background, double t);
    QMap<Background *, BackgroundRenderer *> backgroundRenderers_;

};

#endif
