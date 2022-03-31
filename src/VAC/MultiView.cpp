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

#include "MultiView.h"
#include "View.h"
#include "ViewWidget.h"
#include "Scene.h"
#include "Timeline.h"
#include "Global.h"

#include <QKeyEvent>
#include <QtDebug>
#include <QSplitter>
#include <QVBoxLayout>

// Splitter invariants:
//  1. For every View v, v->parentWidget() is a QSplitter, accessed via getParentSplitter(v)
//  2. For every View v, v is the only child of getParentSplitter(v)
//  3. For every QSplitter s, s has either:
//      a. One child, in which case it is a View, or
//      b. Two children, in which case both of them are QSplitters

namespace
{

ViewWidget * viewWidgetFromView_(View * view)
{
    return qobject_cast<ViewWidget*>(view->parentWidget());
}

View * viewFromViewWidget_(ViewWidget * viewWidget)
{
    return viewWidget->view();
}

QSplitter * getParentSplitter_(QSplitter * w)
{
    return qobject_cast<QSplitter*>(w->parentWidget());
}

QSplitter * getParentSplitter_(ViewWidget * w)
{
    return qobject_cast<QSplitter*>(w->parentWidget());
}

QSplitter * getParentSplitter_(View * w)
{
    return getParentSplitter_(viewWidgetFromView_(w));
}

}

MultiView::MultiView(VPaint::Scene *scene, QWidget *parent) :
    QWidget(parent),
    views_(),
    activeView_(0),
    hoveredView_(0),
    scene_(scene)
{
    // create initial view and splitter
    View * view = createView_();
    QSplitter * splitter = new QSplitter();
    splitter->addWidget(viewWidgetFromView_(view));

    // set layout
    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget(splitter);
    layout->setContentsMargins(0,0,0,0);
    setLayout(layout);

    // Redraw views when necessary
    // This line is necessary since some mouse cursor are drawn by VAC->draw()
    // and depends on which view is hovered, if any
    connect(this, SIGNAL(hoveredViewChanged()), this, SIGNAL(allViewsNeedToUpdate()));
}

View * MultiView::createView_()
{
    ViewWidget * viewWidget = new ViewWidget(scene_, this);
    View * view = viewFromViewWidget_(viewWidget);
    views_ << viewWidget;
    setActiveView(view);
    hoveredView_ = 0;
    Timeline * timeline = global()->timeline();
    if(timeline)
    {
        timeline->addView(view);
    }

    connect(view, SIGNAL(allViewsNeedToUpdate()), this, SIGNAL(allViewsNeedToUpdate()));
    connect(view, SIGNAL(allViewsNeedToUpdatePicking()), this, SIGNAL(allViewsNeedToUpdatePicking()));
    connect(view, SIGNAL(mousePressed(GLWidget*)), this, SLOT(setActive(GLWidget*)));
    connect(view, SIGNAL(mouseEntered(GLWidget*)), this, SLOT(setHovered(GLWidget*)));
    connect(view, SIGNAL(mouseLeft(GLWidget*)), this, SLOT(unsetHovered(GLWidget*)));
    connect(view, SIGNAL(mouseMoved(GLWidget*)), this, SLOT(setHovered(GLWidget*)));
    connect(view, SIGNAL(viewIsBeingChanged(int,int)), this, SIGNAL(cameraChanged()));
    connect(view, SIGNAL(viewChanged(int,int)), this, SIGNAL(cameraChanged()));
    connect(view, SIGNAL(viewResized()), this, SIGNAL(cameraChanged()));
    connect(view, SIGNAL(settingsChanged()), this, SIGNAL(settingsChanged()));

    return view;
}


void MultiView::deleteView_(View * view)
{
    views_.removeAll(viewWidgetFromView_(view));
    Timeline * timeline = global()->timeline();
    if(timeline)
    {
        timeline->removeView(view);
    }
    if(activeView_ == view)
    {
        if(views_.size() > 0)
            setActiveView(viewFromViewWidget_(views_[0]));
        else
            setActiveView(0);
    }
    if(hoveredView_ == view)
    {
        hoveredView_ = 0;
    }
}

void MultiView::setActiveView(View * view)
{
    if(view && (activeView_!=view))
    {
        if(activeView_)
            activeView_->setActive(false);

        activeView_ = view;
        activeView_->setActive(true);

        emit activeViewChanged();
    }
}

void MultiView::showSettingsWidget(bool isShow)
{
    if(activeView_ != nullptr)
    {
        activeView_->viewSettingsWidget()->setVisible(isShow);
    }
}

void MultiView::setActive(GLWidget * w)
{
    View * view =  qobject_cast<View*>(w);
    setActiveView(view);
}

void MultiView::setHovered(GLWidget * w)
{
    View * oldHoveredView =  hoveredView_;

    View * view =  qobject_cast<View*>(w);
    if(view)
    {
        hoveredView_ = view;
    }
    else
    {
        hoveredView_ = 0;
    }

    if(hoveredView_ != oldHoveredView)
        emit hoveredViewChanged();
}

void MultiView::unsetHovered(GLWidget * /*w*/)
{
    View * oldHoveredView =  hoveredView_;

    hoveredView_ = 0;

    if(hoveredView_ != oldHoveredView)
        emit hoveredViewChanged();
}


void MultiView::split_(View * view, Qt::Orientation orientation)
{
    // Schematic view of what happens
    //   _________________________________
    //  | splitterParent                  |
    //  |  _____________________________  |
    //  | | view                        | |
    //  | |                             | |
    //  | |                             | |
    //  | |                             | |
    //  | |                             | |
    //  | |_____________________________| |
    //  |_________________________________|
    //
    //                  []
    //                 _[]_
    //                 \  /
    //                  \/
    //   _________________________________
    //  | splitterParent                  |
    //  |  _____________   _____________  |
    //  | | splitter1   | | splitter2   | |
    //  | |  _________  | |  _________  | |
    //  | | | view    | | | | view2   | | |
    //  | | |         | | | |         | | |
    //  | | |_________| | | |_________| | |
    //  | |_____________| |_____________| |
    //  |_________________________________|
    //

    // Get current splitter and view
    QSplitter * splitterParent = getParentSplitter_(view);
    assert(splitterParent);

    // create view
    View * view2 = createView_();

    // Create two new splitters
    QSplitter * splitter1 = new QSplitter;
    QSplitter * splitter2 = new QSplitter;

    // Change parent widgets
    splitterParent->setOrientation(orientation);
    splitterParent->addWidget(splitter1);
    splitterParent->addWidget(splitter2);
    splitter1->addWidget(viewWidgetFromView_(view)); // This also removes view from splitterParent
    splitter2->addWidget(viewWidgetFromView_(view2));
}

void MultiView::splitClose_(View * view)
{
    assert( numViews() >= 2);

    // Schematic view of what happens
    // Note: [*] is either 1 view, or 2 splitters
    //   _________________________________
    //  | splitterParent                  |
    //  |  _____________   _____________  |
    //  | | splitter1   | | splitter2   | |
    //  | |  _________  | |  _________  | |
    //  | | | view    | | | | *       | | |
    //  | | |         | | | |         | | |
    //  | | |_________| | | |_________| | |
    //  | |_____________| |_____________| |
    //  |_________________________________|
    //
    //                  []
    //                 _[]_
    //                 \  /
    //                  \/
    //   _________________________________
    //  | splitterParent                  |
    //  |  _____________________________  |
    //  | | *                           | |
    //  | |                             | |
    //  | |                             | |
    //  | |                             | |
    //  | |                             | |
    //  | |_____________________________| |
    //  |_________________________________|
    //

    // Get splitters
    QSplitter * splitter1 = getParentSplitter_(view);
    QSplitter * splitterParent = getParentSplitter_(splitter1);
    assert(splitterParent->count() == 2);
    QSplitter * splitter2 = qobject_cast<QSplitter*>(splitterParent->widget(0));
    if(splitter2 == splitter1)
        splitter2 = qobject_cast<QSplitter*>(splitterParent->widget(1));

    // Delete view and splitter1
    // Caution: order is important, because deleteView_(view) does some
    //          management before actually deleting view, and deleting
    //          splitter1 would recursively delete view
    deleteView_(view);
    delete splitter1;

    // Save layout of splitter2
    Qt::Orientation orientation = splitter2->orientation();
    QList<int> sizes = splitter2->sizes();

    // Transfer children of splitter2 to splitterParent
    QList<QWidget*> splitter2Children;
    for(int i=0; i<splitter2->count(); ++i)
        splitter2Children << splitter2->widget(i);
    for(int i=0; i<splitter2Children.size(); ++i)
        splitterParent->addWidget(splitter2Children[i]);

    // Delete splitter2
    delete splitter2;

    // Transfer layout of splitter2 to splitterParent
    splitterParent->setOrientation(orientation);
    splitterParent->setSizes(sizes);
}


void MultiView::split_(Qt::Orientation orientation)
{
    // Get current splitter and view
    View * view1 = activeView();
    QSplitter * s = getParentSplitter_(view1);

    if(s)
    {
        // create view
        View * view2 = createView_();

        // Create two new splitters
        QSplitter * s1 = new QSplitter;
        QSplitter * s2 = new QSplitter;

        // Change parent widgets
        s->setOrientation(orientation);
        s->addWidget(s1);
        s->addWidget(s2);
        s1->addWidget(viewWidgetFromView_(view1)); // This also removes view1 from s
        s2->addWidget(viewWidgetFromView_(view2));
    }
}

void MultiView::splitVertical()
{
    split_(Qt::Horizontal);
}

void MultiView::splitHorizontal()
{
    split_(Qt::Vertical);
}

int MultiView::numViews() const
{
    return views_.size();
}

void MultiView::splitClose()
{
    View * view = activeView();
    if(view && numViews() >= 2)
    {
        splitClose_(view);
    }
}

void MultiView::splitOne()
{
    View * view = activeView();
    if(view && numViews() >= 2)
    {
        QList<ViewWidget*> viewsToClose = views_;
        viewsToClose.removeAll(viewWidgetFromView_(view));
        for(int i=0; i<viewsToClose.size(); ++i)
        {
            splitClose_(viewFromViewWidget_(viewsToClose[i]));
        }
    }
}



View * MultiView::activeView() const
{
    return activeView_;
}

View * MultiView::hoveredView() const
{
    return hoveredView_;
}

MultiView::~MultiView()
{
}

void MultiView::update()
{
    for(ViewWidget * viewWidget: qAsConst(views_))
    {
        View * view = viewFromViewWidget_(viewWidget);
        if(view->isVisible())
            view->update();
    }
}

void MultiView::updatePicking()
{
    for(ViewWidget * viewWidget: qAsConst(views_))
    {
        View * view = viewFromViewWidget_(viewWidget);
        if(view->isVisible())
            view->updatePicking();
    }
}

void MultiView::zoomIn(const double zoomRatio)
{
    activeView()->zoomIn(zoomRatio);
}

void MultiView::zoomOut(const double zoomRatio)
{
    activeView()->zoomOut(zoomRatio);
}

void MultiView::fitAllInWindow()
{
    activeView()->fitAllInWindow();
}

void MultiView::fitSelectionInWindow()
{
    activeView()->fitSelectionInWindow();
}

void MultiView::keyPressEvent(QKeyEvent *event)
{
    emit keyPressed(event);
    event->ignore();
}

void MultiView::keyReleaseEvent(QKeyEvent *event)
{
    emit keyReleased(event);
    event->ignore();
}

// toChange
double MultiView::zoom() const { return activeView()->camera2D().zoom(); }

void MultiView::toggleOutline()
{
    activeView()->toggleOutline();
}

void MultiView::toggleOutlineOnly()
{
    activeView()->toggleOutlineOnly();
}
void MultiView::setDisplayMode(ViewSettings::DisplayMode displayMode)
{
    activeView()->setDisplayMode(displayMode);
}

void MultiView::setOnionSkinningEnabled(bool enabled)
{
    activeView()->setOnionSkinningEnabled(enabled);
}
