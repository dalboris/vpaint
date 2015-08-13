// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef VIEW_3D_H
#define VIEW_3D_H

#include "OpenGL.h"

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

class View3D: public GLWidget
{
    Q_OBJECT
    
public:
    View3D(Scene *scene, QWidget *parent);
    virtual ~View3D();

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

    // Settings widget
    View3DSettingsWidget * view3DSettingsWidget() const;

public slots:
    void update();
    void updatePicking();

    void openViewSettings();
    void closeViewSettings();

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
    View3DSettingsWidget * viewSettingsWidget_;

};

#endif
