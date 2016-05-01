// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef MULTI_VIEW_H
#define MULTI_VIEW_H

#include <QWidget>
#include <QList>
#include <QGridLayout>

class SceneOld;
class ViewOld;
class ViewMacOsX;
class QSplitter;
class GLWidget;

// typedef to solve the MacOSX / Win+Linux discrepency
#ifdef Q_OS_MAC
typedef ViewMacOsX ViewWidget;
#else
typedef ViewOld       ViewWidget;
#endif

#include "ViewSettings.h"

class MultiView: public QWidget
{
    Q_OBJECT
    
public:
    MultiView(SceneOld *scene, QWidget *parent);
    ~MultiView();

    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

    ViewOld * activeView() const;
    ViewOld * hoveredView() const;
    double zoom() const;
    int numViews() const;

    void setActiveView(ViewOld * view);

public slots:
    void update();        // update only the views in MultiView (not the 3D view)
    void updatePicking(); // update only the views in MultiView (not the 3D view)

    void zoomIn();
    void zoomOut();
    void fitAllInWindow();
    void fitSelectionInWindow();

    // Splitting
    void splitVertical();
    void splitHorizontal();
    void splitClose();
    void splitOne();

    // Display mode
    void toggleOutline();
    void toggleOutlineOnly();
    void setDisplayMode(ViewSettings::DisplayMode displayMode);

    void setOnionSkinningEnabled(bool enabled);

signals:
    void allViewsNeedToUpdate();        // update all views (including these and the 3D view)
    void allViewsNeedToUpdatePicking(); // update all views (including these and the 3D view)
    void activeViewChanged();
    void hoveredViewChanged();
    void cameraChanged();
    void settingsChanged();

private slots:
    void setActive(GLWidget * w);
    void setHovered(GLWidget * w);
    void unsetHovered(GLWidget * w);

private:
    // private members
    QList<ViewWidget*> views_;
    ViewOld * activeView_;
    ViewOld * hoveredView_;
    SceneOld * scene_;

    // helper methods
    ViewOld * createView_();
    void deleteView_(ViewOld * view);
    void split_(Qt::Orientation orientation);
    void split_(ViewOld * view, Qt::Orientation orientation);
    void splitClose_(ViewOld * view);
};

#endif
