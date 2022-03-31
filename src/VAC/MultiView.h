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

#ifndef MULTI_VIEW_H
#define MULTI_VIEW_H

#include <QWidget>
#include <QList>
#include <QGridLayout>
#include "VAC/vpaint_global.h"
namespace VPaint
{
class Scene;
}
class View;
class ViewWidget;
class QSplitter;
class GLWidget;

#include "ViewSettings.h"

class Q_VPAINT_EXPORT MultiView: public QWidget
{
    Q_OBJECT
    
public:
    MultiView(VPaint::Scene *scene, QWidget *parent);
    ~MultiView();

    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

    View * activeView() const;
    View * hoveredView() const;
    double zoom() const;
    int numViews() const;

    void setActiveView(View * view);
    void showSettingsWidget(bool isShow);

public slots:
    void update();        // update only the views in MultiView (not the 3D view)
    void updatePicking(); // update only the views in MultiView (not the 3D view)

    void zoomIn(const double zoomRatio = 0.8);
    void zoomOut(const double zoomRatio = 0.8);

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
    void keyPressed(QKeyEvent* event);
    void keyReleased(QKeyEvent* event);

private slots:
    void setActive(GLWidget * w);
    void setHovered(GLWidget * w);
    void unsetHovered(GLWidget * w);

private:
    // private members
    QList<ViewWidget*> views_;
    View * activeView_;
    View * hoveredView_;
    VPaint::Scene * scene_;

    // helper methods
    View * createView_();
    void deleteView_(View * view);
    void split_(Qt::Orientation orientation);
    void split_(View * view, Qt::Orientation orientation);
    void splitClose_(View * view);
};

#endif
