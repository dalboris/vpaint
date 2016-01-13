// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef VIEW_H
#define VIEW_H

#include "OpenGL.h"

#include <cmath>
#include <iostream>
#include "Picking.h"
#include "GLWidget.h"
#include "GeometryUtils.h"
#include <QList>
#include <QPointF>
#include <QPoint>
#include "TimeDef.h"

#include <QImage>
#include <QMap>

#include "ViewSettings.h"

// pre-declarations

class Scene;
namespace VectorAnimationComplex
{
class VAC;
class KeyVertex;
class KeyEdge;
}
class Time;
class Background;

// mouse event in scene coordinates
struct MouseEvent 
{
    // Mouse position
    double x, y;
    // Mouse buttons pressed: at most one of these is true
    bool left, mid, right;
    // Modifiers keys: any number of these can be true
    bool alt, control, shift;
};

class View: public GLWidget
{
    Q_OBJECT
    
public:
    View(Scene *scene, QWidget *parent);
    virtual ~View();

    void initCamera();

    Scene * scene();

    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);

    virtual int decideClicAction();
    virtual int decidePMRAction();
    
    virtual void MoveEvent(double x, double y);
    virtual void ClicEvent(int action, double x, double y);
    virtual void PMRPressEvent(int action, double x, double y);
    virtual void PMRMoveEvent(int action, double x, double y);
    virtual void PMRReleaseEvent(int action, double x, double y);

    virtual void resizeGL(int width, int height);

    virtual void drawScene();

    // picking. Here, x and y are in window coordinates
    void deletePicking();
    Picking::Object getCloserObject(int x, int y);

    // Fit (Not implemented yet)
    void fitAllInWindow();
    void fitSelectionInWindow();

    // Active time
    int activeFrame() const;
    Time activeTime() const;
    void setActiveTime(Time t);

    // Is active
    void setActive(bool isActive);

    void enablePicking();
    void disablePicking();

    // Zoom level
    double zoom() const;

    // Smallest/Biggest scene coordinates visible in the viewport
    double xSceneMin() const;
    double xSceneMax() const;
    double ySceneMin() const;
    double ySceneMax() const;

    ViewSettings viewSettings() const;
    ViewSettingsWidget * viewSettingsWidget() const;

    // This should not belong here (View class), as we may
    // want to draw the scene to an image without having a
    // View opened (e.g., command-line vec->png conversion).
    // In the meantime, that was the easiest way to implement it.
    // Will refactor later.
    QImage drawToImage(double x, double y, double w, double h, int imgW, int imgH, bool transparentBackground = true);
    QImage drawToImage(Time t, double x, double y, double w, double h, int imgW, int imgH, bool transparentBackground = true);

public slots:
    void update();        // update only this view (i.e., redraw the scene, leave other views unchanged)
    void updatePicking(); // update picking for this view only (i.e., redraw the picking image of this view)
    bool updateHoveredObject(int x, int y);
    void handleNewKeyboardModifiers();

    void toggleOutline();
    void toggleOutlineOnly();
    void setDisplayMode(ViewSettings::DisplayMode displayMode);

    void setOnionSkinningEnabled(bool enabled);

    void updateZoomFromView();

protected:
    virtual void resizeEvent(QResizeEvent * event);

signals:
    void allViewsNeedToUpdate();        // update all views (including other 2D or 3D views)
    void allViewsNeedToUpdatePicking(); // update picking of all views (including other 2D or 3D views)

    void settingsChanged();

private slots:
    void clearBackgroundCache_();
    void clearBackgroundCache_(Background * background);

private:
    // What scene to draw
    // Note: which frame to render is specified in viewSettings
    Scene *scene_;

    // Different times might be drawn concurently, either because there are several
    // timeline or a timeline has several time. The method below gives the time to
    // be use for interactivity with the user
    Time interactiveTime() const;
    
    // Mouse event related members
    MouseEvent mouseEvent() const;
    QPoint lastMousePos_;

    // picking
    void newPicking();
    void drawPick();
    uchar * pickingImg(int x, int y);
    GLuint WINDOW_SIZE_X_;
    GLuint WINDOW_SIZE_Y_;
    GLuint textureId_;
    GLuint rboId_;
    GLuint fboId_;
    uchar *pickingImg_;
    Picking::Object hoveredObject_;
    bool pickingIsEnabled_;

    // PMR mouse event temp variables
    int currentAction_;
    double sculptStartRadius_;
    double sculptStartRadius2_;
    double sculptRadiusDx_;
    double sculptRadiusDy_;
    double sculptStartX_;
    double sculptStartY_;

    // Dirty implementation:
    VectorAnimationComplex::VAC * vac_;
    VectorAnimationComplex::KeyVertex * ivertex_;
    VectorAnimationComplex::KeyEdge * iedge_;

    // View Settings
    ViewSettings viewSettings_;
    ViewSettingsWidget * viewSettingsWidget_;

    // Draw background
    // For now, there's only one background per scene, but we anticipate the
    // case where there is one background per layer, reason why we have
    //     QMap<const Background *, QMap<int,GLuint> >
    // instead of simply
    //     QMap<int,GLuint>
    // See also comment in the implementation of clearBackgroundCache_()
    void drawBackground_(Background * background, int frame);
    GLuint backgroundTexId_(Background * background, int frame);
    QMap<Background *, QMap<int,GLuint> > backgroundTexIds_;
};

#endif
