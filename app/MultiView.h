// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef MULTI_VIEW_H
#define MULTI_VIEW_H

#include <QWidget>
#include <QList>
#include <QGridLayout>

class Scene;
class View;
class ViewMacOsX;
class QSplitter;
class GLWidget;

// typedef to solve the MacOSX / Win+Linux discrepency
#ifdef Q_OS_MAC
typedef ViewMacOsX ViewWidget;
#else
typedef View       ViewWidget;
#endif

#include "ViewSettings.h"

class MultiView: public QWidget
{
    Q_OBJECT
    
public:
    MultiView(Scene *scene, QWidget *parent);
    ~MultiView();

    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

    View * activeView() const;
    View * hoveredView() const;
    double zoom() const;
    int numViews() const;

    void setActiveView(View * view);

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
    View * activeView_;
    View * hoveredView_;
    Scene * scene_;

    // helper methods
    View * createView_();
    void deleteView_(View * view);
    void split_(Qt::Orientation orientation);
    void split_(View * view, Qt::Orientation orientation);
    void splitClose_(View * view);
};

#endif
