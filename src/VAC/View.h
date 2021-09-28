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

#ifndef VIEW_H
#define VIEW_H

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
#include "vpaint_global.h"

class Scene;
namespace VectorAnimationComplex
{
class VAC;
class KeyVertex;
class KeyEdge;
}
class Time;
class Background;
class BackgroundRenderer;

// mouse event in scene coordinates
struct Q_VPAINT_EXPORT MouseEvent
{
    // Mouse position
    double x, y;
    // Mouse buttons pressed: at most one of these is true
    bool left, mid, right;
    // Modifiers keys: any number of these can be true
    bool alt, control, shift;
};

class Q_VPAINT_EXPORT View: public GLWidget
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
    QImage drawToImage(double x, double y, double w, double h, int imgW, int imgH, bool useViewSettings);
    QImage drawToImage(Time t, double x, double y, double w, double h, int imgW, int imgH, bool useViewSettings);

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

    void drawSceneDelegate_(Time t);

protected:
    virtual void resizeEvent(QResizeEvent * event);

signals:
    void allViewsNeedToUpdate();        // update all views (including other 2D or 3D views)
    void allViewsNeedToUpdatePicking(); // update picking of all views (including other 2D or 3D views)

    void settingsChanged();

private slots:
    void onBackgroundDestroyed_(Background * background);

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
    GLsizei pickingWidth_;
    GLsizei pickingHeight_;
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
    // We use a map of BackgroundRenderer to anticipate the moment we have more
    // than one Background (i.e., one per layer)
    BackgroundRenderer * getBackgroundRenderer_(Background * background);
    BackgroundRenderer * createBackgroundRenderer_(Background * background);
    void destroyBackgroundRenderer_(Background * background);
    BackgroundRenderer * getOrCreateBackgroundRenderer_(Background * background);
    void drawBackground_(Background * background, int frame);
    QMap<Background *, BackgroundRenderer *> backgroundRenderers_;
};

#endif
