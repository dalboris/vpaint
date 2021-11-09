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

#include "AnimatedCycleWidget.h"
#include "VectorAnimationComplex/InbetweenCell.h"
#include "VectorAnimationComplex/KeyCell.h"
#include "VectorAnimationComplex/InbetweenEdge.h"
#include "VectorAnimationComplex/InbetweenFace.h"
#include "VectorAnimationComplex/KeyEdge.h"
#include "VectorAnimationComplex/VAC.h"

#include <QVBoxLayout>
#include <QVector2D>
#include <QGraphicsSceneMouseEvent>
#include <QMouseEvent>
#include <QPushButton>

#include "Global.h"
#include "MainWindow.h"
#include "Scene.h"

#include "Random.h"

#include <QtDebug>

namespace
{
const double ARROW_LENGTH = 30;
}

GraphicsNodeItem::GraphicsNodeItem(AnimatedCycleNode * node, AnimatedCycleWidget * widget) :
    node_(node),
    widget_(widget),
    isMoved_(false)
{
    // Set path and color
    if(node_->cell()->toKeyVertex())
    {
        setBrush(QColor(255,170,170));
        width_ =  20;
        height_ = 20;
    }
    else if(node_->cell()->toKeyEdge())
    {
        setBrush(QColor(170,204,255));
        width_ =  60;
        height_ = 20;
    }
    else if(node_->cell()->toInbetweenVertex())
    {
        setBrush(QColor(255,218,218));
        width_ =  20;
        height_ = 60;
    }
    else if(node_->cell()->toInbetweenEdge())
    {
        setBrush(QColor(235,243,255));
        width_ =  60;
        height_ = 60;
    }
    setPath_();

    // Create text label as a child item
    text_ = new QGraphicsTextItem(QString(), this);
    text_->setFont(QFont("arial",7));
    updateText();

    // Moake item movable
    setFlag(ItemIsMovable, true);

    // Observe cell
    observe(node_->cell());
}

GraphicsNodeItem::~GraphicsNodeItem()
{
    // Unobserve cell
    unobserve(node_->cell());
}

void GraphicsNodeItem::observedCellDeleted(Cell *)
{
    // Commit suicide properly
    widget_->deleteItem(this);
}

void GraphicsNodeItem::updateText()
{
    QString string;
    string.setNum(node_->cell()->id());
    if(node_->cell()->toEdgeCell())
        (node_->side()) ? (string += "+") : (string += "-");

    text_->setPlainText(string);

    double textWidth = text_->boundingRect().width();
    double textHeight = text_->boundingRect().height();
    text_->setPos(-0.5*textWidth,-0.5*textHeight);
}

void GraphicsNodeItem::setPath_()
{
    QPainterPath path;
    EdgeCell * edge = node()->cell()->toEdgeCell();
    if(edge && edge->isClosed())
    {
        path.moveTo(-0.5*width_-10,-0.5*height_); // A
        path.lineTo(0.5*width_-10,-0.5*height_); // B
        path.cubicTo(0.5*width_-5,-0.5*height_,0.5*width_,-0.5*height_+5,0.5*width_,-0.5*height_+10); // C
        path.lineTo(0.5*width_,0.5*height_-10); // D
        path.cubicTo(0.5*width_,0.5*height_-5,0.5*width_-5,0.5*height_,0.5*width_-10,0.5*height_); //E
        path.lineTo(-0.5*width_-10,0.5*height_); // F
        path.cubicTo(-0.5*width_-5,0.5*height_,-0.5*width_,0.5*height_-5,-0.5*width_,0.5*height_-10); // G
        path.lineTo(-0.5*width_,-0.5*height_+10); // H
        path.cubicTo(-0.5*width_,-0.5*height_+5,-0.5*width_-5,-0.5*height_,-0.5*width_-10,-0.5*height_); // I
    }
    else
    {
        path.addRoundedRect(-0.5*width_,-0.5*height_,width_,height_,10,10);
    }
    setPath(path);
}

double GraphicsNodeItem::width() const
{
    return width_;
}

double GraphicsNodeItem::height() const
{
    return height_;
}

void GraphicsNodeItem::setWidth(double w)
{
    width_ = w;
    setPath_();
}

void GraphicsNodeItem::setHeight(int i)
{
    height_ = i*60 + (i-1)*(20+2*ARROW_LENGTH);
    setPath_();
}

void GraphicsNodeItem::setFixedY(double y)
{
    y_ = y;
    setY(y_);
}

void GraphicsNodeItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    if(event->button() == Qt::LeftButton)
    {
        isMoved_ = true;
        QGraphicsPathItem::mousePressEvent(event);
    }
    else if(event->button() == Qt::RightButton)
    {
        //qDebug() << "press" << node()->cell()->id();
    }
    else
    {
        QGraphicsPathItem::mousePressEvent(event);
    }
}

void GraphicsNodeItem::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    QGraphicsPathItem::mouseMoveEvent(event);
    double eps = 1.0e-4;
    double delta = y_ - y();
    if( delta < -eps || eps < delta )
        setY(y_);
}

void GraphicsNodeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    if(event->button() == Qt::LeftButton)
    {
        isMoved_ = false;
        QGraphicsPathItem::mouseReleaseEvent(event);
    }
    else if (event->button() == Qt::RightButton)
    {
        //QGraphicsItem * item = scene()->itemAt(event->pos(), view)
        //qDebug() << "release" << node()->cell()->id();
    }
    else
    {
        QGraphicsPathItem::mouseReleaseEvent(event);
    }
}

bool GraphicsNodeItem::isMoved() const
{
    return isMoved_;
}

AnimatedCycleNode * GraphicsNodeItem::node() const
{
    return node_;
}

GraphicsNodeItem * GraphicsNodeItem::next()
{
    return widget_->next(this);
}
GraphicsNodeItem * GraphicsNodeItem::previous()
{
    return widget_->previous(this);
}
GraphicsNodeItem * GraphicsNodeItem::before()
{
    return widget_->before(this);
}
GraphicsNodeItem * GraphicsNodeItem::after()
{
    return widget_->after(this);
}



GraphicsArrowItem::GraphicsArrowItem() :
    QGraphicsPolygonItem()
{
    setPen(QPen(Qt::black));
    setBrush(QBrush(Qt::black));
}

void GraphicsArrowItem::setEndPoints(const QPointF & p1, const QPointF & p2)
{
    QPolygonF polygon;
    QVector2D u(p2-p1);
    u.normalize();
    QVector2D v(-u.y(),u.x());

    double lineHalfWidth = 1;
    double arrowHalfWidth = 5;
    double arrowLength = 5;

    polygon << p1 + (lineHalfWidth*v).toPointF()
            << p2 + (lineHalfWidth*v - arrowLength*u).toPointF()
            << p2 + (arrowHalfWidth*v - arrowLength*u).toPointF()
            << p2
            << p2 + (-arrowHalfWidth*v - arrowLength*u).toPointF()
            << p2 + (-lineHalfWidth*v - arrowLength*u).toPointF()
            << p1 + (-lineHalfWidth*v).toPointF();

    setPolygon(polygon);
}






AnimatedCycleGraphicsView::AnimatedCycleGraphicsView(QGraphicsScene * scene, AnimatedCycleWidget * animatedCycleWidget) :
    QGraphicsView(scene),
    animatedCycleWidget_(animatedCycleWidget)
{
    //setTransformationAnchor(AnchorUnderMouse);
}

void AnimatedCycleGraphicsView::paintEvent(QPaintEvent * event)
{
    QGraphicsView::paintEvent(event);
}

void AnimatedCycleGraphicsView::wheelEvent(QWheelEvent *event)
{
    setTransformationAnchor(AnchorUnderMouse);
    double ratio = 1.0 / pow( 0.8f, (double) event->delta() / (double) 120.0f);
    scale(ratio, ratio);
}

GraphicsNodeItem * AnimatedCycleGraphicsView::nodeItemAt(const QPoint & pos)
{
    GraphicsNodeItem * res = 0;

    QGraphicsItem * item = itemAt(pos);
    GraphicsNodeItem * nodeItem = qgraphicsitem_cast<GraphicsNodeItem*>(item);
    GraphicsArrowItem * arrowItem = qgraphicsitem_cast<GraphicsArrowItem*>(item);
    QGraphicsTextItem * textItem = qgraphicsitem_cast<QGraphicsTextItem*>(item);

    if(nodeItem)
    {
        res = nodeItem;
    }
    else if(arrowItem)
    {
        res = 0;
    }
    else if(textItem)
    {
        item = item->parentItem();
        nodeItem = qgraphicsitem_cast<GraphicsNodeItem*>(item);
        if(nodeItem)
        {
            res = nodeItem;
        }
        else
        {
            res = 0;
        }
    }

    return res;
}

GraphicsArrowItem * AnimatedCycleGraphicsView::arrowItemAt(const QPoint & pos)
{
    return qgraphicsitem_cast<GraphicsArrowItem*>(itemAt(pos));
}

void AnimatedCycleGraphicsView::mousePressEvent(QMouseEvent * event)
{
    if(event->button() == Qt::MidButton)
    {
        setTransformationAnchor(AnchorUnderMouse);
        setInteractive(false);
        setDragMode(ScrollHandDrag);
        QMouseEvent fake(event->type(), event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());
        QGraphicsView::mousePressEvent(&fake);
    }
    else if(event->button() == Qt::LeftButton && global()->keyboardModifiers().testFlag(Qt::AltModifier))
    {
        GraphicsArrowItem * arrowItem = arrowItemAt(event->pos());
        GraphicsNodeItem * nodeItem = nodeItemAt(event->pos());
        if(arrowItem)
        {
            animatedCycleWidget_->deleteArrow(arrowItem);
            animatedCycleWidget_->save();
        }
        else if(nodeItem)
        {
            animatedCycleWidget_->deleteItem(nodeItem);
            animatedCycleWidget_->save();
        }
    }
    else if(event->button() == Qt::LeftButton && global()->keyboardModifiers().testFlag(Qt::ControlModifier))
    {
        GraphicsNodeItem * nodeItem = nodeItemAt(event->pos());
        if(nodeItem)
        {
            nodeItem->node()->setSide(!nodeItem->node()->side());
            nodeItem->updateText();
            animatedCycleWidget_->save();
        }
    }
    else if(event->button() == Qt::RightButton)
    {
        itemAtPress_ = nodeItemAt(event->pos());
    }
    else
    {
        QGraphicsView::mousePressEvent(event);
    }
}

void AnimatedCycleGraphicsView::mouseMoveEvent(QMouseEvent * event)
{
    QGraphicsView::mouseMoveEvent(event);
}

void AnimatedCycleGraphicsView::mouseReleaseEvent(QMouseEvent * event)
{
    if(event->button() == Qt::MidButton)
    {
        QMouseEvent fake(event->type(), event->pos(), Qt::LeftButton, Qt::LeftButton, event->modifiers());
        QGraphicsView::mouseReleaseEvent(&fake);
        setDragMode(NoDrag);
        setInteractive(true);
    }
    else if(event->button() == Qt::RightButton)
    {
        GraphicsNodeItem * item = nodeItemAt(event->pos());
        if(item && itemAtPress_)
        {
            InbetweenCell * pressedInbetween = itemAtPress_->node()->cell()->toInbetweenCell();
            InbetweenCell * releasedInbetween = item->node()->cell()->toInbetweenCell();

            KeyCell * pressedKey = itemAtPress_->node()->cell()->toKeyCell();
            KeyCell * releasedKey = item->node()->cell()->toKeyCell();

            const int PREVIOUS = 0;
            const int NEXT = 1;
            const int BEFORE = 2;
            const int AFTER = 3;
            const int PREVIOUS_OR_NEXT = 4;
            int arrowType = PREVIOUS;

            // Determine arrow type
            if( (pressedKey && releasedKey) || (pressedInbetween && releasedInbetween) )
            {
                arrowType = PREVIOUS_OR_NEXT;
            }
            else if (pressedKey && releasedInbetween)
            {
                int t = pressedKey->time().frame();
                int t1 = releasedInbetween->beforeTime().frame();
                int t2 = releasedInbetween->afterTime().frame();

                if(t <= t1)
                    arrowType = AFTER;
                else if(t >= t2)
                    arrowType = BEFORE;
                else
                    arrowType = PREVIOUS_OR_NEXT;
            }
            else
            {
                assert(pressedInbetween && releasedKey);

                int t = releasedKey->time().frame();
                int t1 = pressedInbetween->beforeTime().frame();
                int t2 = pressedInbetween->afterTime().frame();

                if(t <= t1)
                    arrowType = BEFORE;
                else if(t >= t2)
                    arrowType = AFTER;
                else
                    arrowType = PREVIOUS_OR_NEXT;
            }

            // set pointer
            if(arrowType == PREVIOUS_OR_NEXT)
            {
                if(global()->keyboardModifiers().testFlag(Qt::ControlModifier))
                    arrowType = PREVIOUS;
                else
                    arrowType = NEXT;
            }
            if(arrowType == PREVIOUS)
                animatedCycleWidget_->setPrevious(itemAtPress_,item);
            else if(arrowType == NEXT)
                animatedCycleWidget_->setNext(itemAtPress_,item);
            else if(arrowType == BEFORE)
                animatedCycleWidget_->setBefore(itemAtPress_,item);
            else if(arrowType == AFTER)
                animatedCycleWidget_->setAfter(itemAtPress_,item);


            animatedCycleWidget_->save();
        }
    }
    else
    {
        QGraphicsView::mouseReleaseEvent(event);
    }
}

AnimatedCycleWidget::AnimatedCycleWidget(QWidget *parent) :
    QWidget(parent),
    isReadOnly_(true),
    inbetweenFace_(0)
{
    scene_ = new QGraphicsScene();
    view_ = new AnimatedCycleGraphicsView(scene_, this);
    view_->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QPushButton * addSelectedCellsButton = new QPushButton("add selected cells");
    QPushButton * saveButton = new QPushButton("save");
    QPushButton * loadButton = new QPushButton("load");
    connect(addSelectedCellsButton, SIGNAL(clicked()), this, SLOT(addSelectedCells()));
    connect(saveButton, SIGNAL(clicked()), this, SLOT(save()));
    connect(loadButton, SIGNAL(clicked()), this, SLOT(load()));
    editorButtons_ = new QHBoxLayout();
    editorButtons_->addWidget(addSelectedCellsButton);
    //editorButtons_->addWidget(saveButton);
    editorButtons_->addWidget(loadButton);


    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget(view_);
    layout->addLayout(editorButtons_);
    setLayout(layout);

    timer_.setInterval(16);
    connect(&timer_, SIGNAL(timeout()), this, SLOT(animate()));
}


void AnimatedCycleWidget::createNodeAndItem(Cell * cell)
{
    AnimatedCycleNode * node = new AnimatedCycleNode(cell);
    if(!animatedCycle_.first())
    {
        animatedCycle_.setFirst(node);

        // Caution: the line below would cause a crash, as node ownership is transfered
        //          to the temp object AnimatedCycle(node) which is destroyed after the copy occured.
        //animatedCycle_ = AnimatedCycle(node);
    }

    createItem(node);
}

void AnimatedCycleWidget::createItem(AnimatedCycleNode * node)
{
    GraphicsNodeItem * item = new GraphicsNodeItem(node, this);
    scene_->addItem(item);
    nodeToItem_[node] = item;
}

void AnimatedCycleWidget::addSelectedCells()
{
    VectorAnimationComplex::VAC * vac = global()->mainWindow()->scene()->activeVAC();
    if (vac)
    {
        CellSet selectedCells = vac->selectedCells();

        for(Cell * cell: selectedCells)
            createNodeAndItem(cell);

        computeItemHeightAndY();

        save();
    }
}


void AnimatedCycleWidget::start()
{
    timer_.start();
}

void AnimatedCycleWidget::stop()
{
    timer_.stop();
}

AnimatedCycleWidget::~AnimatedCycleWidget()
{
    clearAnimatedCycle(); // important: cells must be unobserved
    delete scene_;
}

void AnimatedCycleWidget::setReadOnly(bool b)
{
    isReadOnly_ = b;
}

bool AnimatedCycleWidget::isReadOnly() const
{
    return isReadOnly_;
}

void AnimatedCycleWidget::clearScene()
{
    // Delete all items
    view_->setScene(0);
    delete scene_;

    // Create new empty scene
    scene_ = new QGraphicsScene();

    // Set scene properties
    view_->setTransformationAnchor(QGraphicsView::NoAnchor);
    view_->setScene(scene_);

    // Clear maps and sets
    nodeToItem_.clear();
    nodeToItem_[0] = 0;
    itemToNextArrow_.clear();
    itemToPreviousArrow_.clear();
    itemToNextArrowBorder_.clear();
    itemToPreviousArrowBorder_.clear();
    itemToAfterArrow_.clear();
    itemToBeforeArrow_.clear();
}

void AnimatedCycleWidget::observedCellDeleted(Cell *)
{
    clearAnimatedCycle();
}

void AnimatedCycleWidget::clearAnimatedCycle()
{
    // Break connection between widget and vac
    if(inbetweenFace_)
    {
        unobserve(inbetweenFace_);
        inbetweenFace_ = 0;
    }

    // Clear scene
    clearScene();
}

void AnimatedCycleWidget::setAnimatedCycle(InbetweenFace * inbetweenFace, int indexCycle)
{
    // Clear
    clearAnimatedCycle();

    // Set new animated cycle
    if(inbetweenFace && indexCycle >= 0 && indexCycle < inbetweenFace->numAnimatedCycles())
    {
        inbetweenFace_ = inbetweenFace;
        indexCycle_ = indexCycle;

        observe(inbetweenFace_);

        load();
    }
}

void AnimatedCycleWidget::setAnimatedCycle(const AnimatedCycle & animatedCycle)
{
    // Clear scene
    clearAnimatedCycle();

    // Set new animated cycle
    animatedCycle_ = animatedCycle;

    // Create items
    computeSceneFromAnimatedCycle();
}

void AnimatedCycleWidget::load()
{
    clearScene();

    if(inbetweenFace_ && indexCycle_ >= 0 && indexCycle_ < inbetweenFace_->numAnimatedCycles())
    {
        animatedCycle_ = inbetweenFace_->animatedCycle(indexCycle_);
        computeSceneFromAnimatedCycle();
        start();
    }
}

void AnimatedCycleWidget::save()
{
    if(inbetweenFace_ && indexCycle_ >= 0 && indexCycle_ < inbetweenFace_->numAnimatedCycles())
    {
        inbetweenFace_->setCycle(indexCycle_,animatedCycle_);

        VectorAnimationComplex::VAC * vac = inbetweenFace_->vac();
        emit vac->needUpdatePicking();
        emit vac->changed();
        emit vac->checkpoint();
    }
}

AnimatedCycle AnimatedCycleWidget::getAnimatedCycle() const
{
    return animatedCycle_;
}


void AnimatedCycleWidget::setBefore(GraphicsNodeItem * item, GraphicsNodeItem * itemBefore)
{
    // Delete existing arrow
    if(itemToBeforeArrow_.contains(item))
    {
        delete itemToBeforeArrow_[item];
        itemToBeforeArrow_.remove(item);
    }

    // Set node pointer value
    item->node()->setBefore(itemBefore->node());

    // Create arrow
    GraphicsArrowItem * arrow = new GraphicsArrowItem();
    scene_->addItem(arrow);
    itemToBeforeArrow_[item] = arrow;
}

void AnimatedCycleWidget::setAfter(GraphicsNodeItem * item, GraphicsNodeItem * itemAfter)
{
    // Delete existing arrow
    if(itemToAfterArrow_.contains(item))
    {
        delete itemToAfterArrow_[item];
        itemToAfterArrow_.remove(item);
    }

    // Set node pointer value
    item->node()->setAfter(itemAfter->node());

    // Create arrow
    GraphicsArrowItem * arrow = new GraphicsArrowItem();
    scene_->addItem(arrow);
    itemToAfterArrow_[item] = arrow;
}

void AnimatedCycleWidget::setPrevious(GraphicsNodeItem * item, GraphicsNodeItem * itemPrevious)
{
    // Delete existing arrow
    if(itemToPreviousArrow_.contains(item))
    {
        delete itemToPreviousArrow_[item];
        itemToPreviousArrow_.remove(item);
    }
    if(itemToPreviousArrowBorder_.contains(item))
    {
        delete itemToPreviousArrowBorder_[item];
        itemToPreviousArrowBorder_.remove(item);
    }

    // Set node pointer value
    item->node()->setPrevious(itemPrevious->node());

    // Create arrow
    GraphicsArrowItem * arrow = new GraphicsArrowItem();
    scene_->addItem(arrow);
    Qt::KeyboardModifiers modifiers = global()->keyboardModifiers();
    if(modifiers.testFlag(Qt::AltModifier))
        itemToPreviousArrowBorder_[item] = arrow;
    else
        itemToPreviousArrow_[item] = arrow;
}

void AnimatedCycleWidget::setNext(GraphicsNodeItem * item, GraphicsNodeItem * itemNext)
{
    // Delete existing arrow
    if(itemToNextArrow_.contains(item))
    {
        delete itemToNextArrow_[item];
        itemToNextArrow_.remove(item);
    }
    if(itemToNextArrowBorder_.contains(item))
    {
        delete itemToNextArrowBorder_[item];
        itemToNextArrowBorder_.remove(item);
    }

    // Set node pointer value
    item->node()->setNext(itemNext->node());

    // Create arrow
    GraphicsArrowItem * arrow = new GraphicsArrowItem();
    scene_->addItem(arrow);
    Qt::KeyboardModifiers modifiers = global()->keyboardModifiers();
    if(modifiers.testFlag(Qt::AltModifier))
        itemToNextArrowBorder_[item] = arrow;
    else
        itemToNextArrow_[item] = arrow;
}

void AnimatedCycleWidget::deleteArrow(GraphicsArrowItem * arrowItem)
{
    typedef QMap<GraphicsNodeItem*, GraphicsArrowItem*> Map;
    typedef Map::iterator Iterator;
    typedef Map* MapPtr;

    const int NUM_MAPS = 6;
    MapPtr maps[NUM_MAPS] = { &itemToNextArrow_ ,
                              &itemToPreviousArrow_ ,
                              &itemToNextArrowBorder_ ,
                              &itemToPreviousArrowBorder_ ,
                              &itemToAfterArrow_ ,
                              &itemToBeforeArrow_ };

    for(int i=0; i<NUM_MAPS; ++i)
    {
        for(Iterator it=maps[i]->begin(); it!=maps[i]->end(); ++it)
        {
            if(it.value() == arrowItem)
            {
                if(i==0 || i==2)
                    it.key()->node()->setNext(0);
                else if(i==1 || i==3)
                    it.key()->node()->setPrevious(0);
                else if(i==4)
                    it.key()->node()->setAfter(0);
                else if(i==5)
                    it.key()->node()->setBefore(0);

                maps[i]->erase(it);
                delete arrowItem;
                return;
            }
        }
    }
}

void AnimatedCycleWidget::deleteItem(GraphicsNodeItem * item)
{
    // Get node corresponding to item
    AnimatedCycleNode * node = item->node();

    // Get arrows starting or ending at item
    QSet<GraphicsArrowItem*> arrowItems;
    typedef QMap<GraphicsNodeItem*, GraphicsArrowItem*> Map;
    typedef Map::iterator Iterator;
    typedef Map* MapPtr;
    const int NUM_MAPS = 6;
    MapPtr maps[NUM_MAPS] = { &itemToNextArrow_ ,
                              &itemToPreviousArrow_ ,
                              &itemToNextArrowBorder_ ,
                              &itemToPreviousArrowBorder_ ,
                              &itemToAfterArrow_ ,
                              &itemToBeforeArrow_ };
    for(int i=0; i<NUM_MAPS; ++i)
    {
        for(Iterator it=maps[i]->begin(); it!=maps[i]->end(); ++it)
        {
            // Get start-arrow-end
            AnimatedCycleNode * startNode = it.key()->node();
            GraphicsArrowItem * arrowItem = it.value();
            AnimatedCycleNode * endNode = 0;
            if(i==0 || i==2)
                endNode = startNode->next();
            else if(i==1 || i==3)
                endNode = startNode->previous();
            else if(i==4)
                endNode = startNode->after();
            else if(i==5)
                endNode = startNode->before();

            // Determine if arrow should be deleted
            if(startNode == node || endNode == node)
                arrowItems << arrowItem;
        }
    }

    // Delete arrows starting or ending at item
    for(GraphicsArrowItem * arrowItem: arrowItems)
        deleteArrow(arrowItem);

    // Delete node and item
    scene_->removeItem(item);
    nodeToItem_.remove(node);
    delete item; // must be first, since it calls node->cell()
    delete node;

    // Update "first" node of animated cycle
    if(animatedCycle_.first() == node)
    {
        QMap<AnimatedCycleNode*, GraphicsNodeItem*>::iterator it = nodeToItem_.begin();

        if(it == nodeToItem_.end())
        {
            animatedCycle_.setFirst(0);
        }
        else
        {
            AnimatedCycleNode * first = it.key();
            if(!first)
            {
                ++it;
                if(it != nodeToItem_.end())
                    first = it.key();
            }

            while(first && first->before())
                first = first->before();

            animatedCycle_.setFirst(first);
        }
    }
}

void AnimatedCycleWidget::computeSceneFromAnimatedCycle()
{
    // Clear scene
    clearScene();

    // Get start nodes
    QSet<AnimatedCycleNode*> startNodes;
    AnimatedCycleNode * startNode = animatedCycle_.first();
    while(startNode)
    {
        startNodes << startNode;
        startNode = startNode->after();
    }

    // Create items
    for(AnimatedCycleNode * node: animatedCycle_.nodes())
    {
        GraphicsNodeItem * item = new GraphicsNodeItem(node, this);
        scene_->addItem(item);
        nodeToItem_[node] = item;
    }

    // Set item height and Y
    computeItemHeightAndY();

    // Create arrows
    for(AnimatedCycleNode * node: animatedCycle_.nodes())
    {
        GraphicsNodeItem * item = nodeToItem_[node];

        if(node->next()) // can be false if cycle is invalid
        {
            GraphicsArrowItem * arrow = new GraphicsArrowItem();
            scene_->addItem(arrow);
            if(!startNodes.contains(node->next()))
                itemToNextArrow_[item] = arrow;
            else
                itemToNextArrowBorder_[item] = arrow;
        }

        if(node->previous()) // can be false if cycle is invalid
        {
            GraphicsArrowItem * arrow = new GraphicsArrowItem();
            scene_->addItem(arrow);
            if(!startNodes.contains(node))
                itemToPreviousArrow_[item] = arrow;
            else
                itemToPreviousArrowBorder_[item] = arrow;
        }

        if(node->after())
        {
            GraphicsArrowItem * arrow = new GraphicsArrowItem();
            scene_->addItem(arrow);
            itemToAfterArrow_[item] = arrow;
        }

        if(node->before())
        {
            GraphicsArrowItem * arrow = new GraphicsArrowItem();
            scene_->addItem(arrow);
            itemToBeforeArrow_[item] = arrow;
        }
    }
}

void AnimatedCycleWidget::computeItemHeightAndY()
{
    typedef QMap<AnimatedCycleNode*, GraphicsNodeItem*>::iterator Iterator;

    // Get key times
    QSet<int> keyTimes;
    for(Iterator it = nodeToItem_.begin(); it != nodeToItem_.end(); ++it)
    {
        AnimatedCycleNode * node = it.key();
        if(!node)
            continue;

        Cell * cell = node->cell();
        InbetweenCell * inbetweenCell = cell->toInbetweenCell();

        if(inbetweenCell)
        {
            int tBefore = inbetweenCell->beforeTime().frame();
            int tAfter = inbetweenCell->afterTime().frame();
            keyTimes << tBefore << tAfter;
        }
        else
        {
            int t = cell->toKeyCell()->time().frame();
            keyTimes << t;
        }
    }

    // Sort key times and compute height and Y of items
    QList<int> keyTimesSorted = keyTimes.toList();
    qSort(keyTimesSorted);
    for(Iterator it = nodeToItem_.begin(); it != nodeToItem_.end(); ++it)
    {
        AnimatedCycleNode * node = it.key();
        if(!node)
            continue;

        GraphicsNodeItem * item = it.value();
        Cell * cell = node->cell();
        InbetweenCell * inbetweenCell = cell->toInbetweenCell();

        if(inbetweenCell)
        {
            int tBefore = inbetweenCell->beforeTime().frame();
            int tAfter = inbetweenCell->afterTime().frame();

            int idBefore = 0;
            int idAfter = 0;
            for(int i=0; i<keyTimesSorted.size(); ++i)
            {
                if(keyTimesSorted[i]==tBefore)
                    idBefore = i;

                if(keyTimesSorted[i]==tAfter)
                    idAfter = i;
            }

            item->setHeight(idAfter-idBefore);

            double yBefore = idBefore * (80 + 2*ARROW_LENGTH);
            double yAfter = idAfter * (80 + 2*ARROW_LENGTH);

            item->setFixedY(0.5 * (yAfter + yBefore));
        }
        else
        {
            int t = cell->toKeyCell()->time().frame();

            int id = 0;
            for(int i=0; i<keyTimesSorted.size(); ++i)
            {
                if(keyTimesSorted[i]==t)
                    id = i;
            }

            item->setFixedY(id * (80 + 2*ARROW_LENGTH));
        }
    }
}

GraphicsNodeItem * AnimatedCycleWidget::next(GraphicsNodeItem * item)
{
    return nodeToItem_[item->node()->next()];
}
GraphicsNodeItem * AnimatedCycleWidget::previous(GraphicsNodeItem * item)
{
    return nodeToItem_[item->node()->previous()];
}
GraphicsNodeItem * AnimatedCycleWidget::before(GraphicsNodeItem * item)
{
    return nodeToItem_[item->node()->before()];
}
GraphicsNodeItem * AnimatedCycleWidget::after(GraphicsNodeItem * item)
{
    return nodeToItem_[item->node()->after()];
}

void AnimatedCycleWidget::mousePressEvent(QMouseEvent * event)
{
    QWidget::mousePressEvent(event);
}

void AnimatedCycleWidget::animate()
{
    // Compute delta between current and target
    QMap<GraphicsNodeItem*, double> deltaX;
    QMap<GraphicsNodeItem*, int> deltaXNum;
    QMap<GraphicsNodeItem*, double> deltaMinX;
    QMap<GraphicsNodeItem*, double> deltaMaxX;

    // Get all items
    QSet<GraphicsNodeItem*> items;
    for(QGraphicsItem * i: scene_->items())
    {
        GraphicsNodeItem * item = qgraphicsitem_cast<GraphicsNodeItem *>(i);
        if(item)
            items << item;
    }

    // Initialize values
    for(GraphicsNodeItem * item: items)
    {
        deltaX[item] = 0;
        deltaXNum[item] = 0;
        deltaMinX[item] = -10000;// std::numeric_limits<double>::lowest();
        deltaMaxX[item] = 10000;// std::numeric_limits<double>::max();
    }

    // Next arrow contribution
    QMapIterator<GraphicsNodeItem*, GraphicsArrowItem*> it(itemToNextArrow_);
    while(it.hasNext())
    {
        it.next();

        GraphicsNodeItem * item = it.key();
        GraphicsNodeItem * nextItem = item->next();

        if(!item || !nextItem)
            continue;

        double start = item->pos().x() + 0.5*item->width();
        double end = nextItem->pos().x() - 0.5*nextItem->width();
        double vec = end - start;
        double delta = ARROW_LENGTH - vec;

        //deltaX[nextItem] += delta;
        //deltaXNum[nextItem] += 1;
        deltaMinX[nextItem] = std::max(delta,deltaMinX[nextItem]);

        //deltaX[item] += -delta;
        //deltaXNum[item] += 1;
        deltaMaxX[item] = std::min(-delta,deltaMaxX[item]);
    }

    // Previous arrow contribution
    it = QMapIterator<GraphicsNodeItem*, GraphicsArrowItem*>(itemToPreviousArrow_);
    while(it.hasNext())
    {
        it.next();

        GraphicsNodeItem * nextItem = it.key();
        GraphicsNodeItem * item = nextItem->previous();

        if(!item || !nextItem)
            continue;

        double start = item->pos().x() + 0.5*item->width();
        double end = nextItem->pos().x() - 0.5*nextItem->width();
        double vec = end - start;
        double delta = ARROW_LENGTH - vec;

        //deltaX[nextItem] += delta;
        //deltaXNum[nextItem] += 1;
        deltaMinX[nextItem] = std::max(delta,deltaMinX[nextItem]);

        //deltaX[item] += -delta;
        //deltaXNum[item] += 1;
        deltaMaxX[item] = std::min(-delta,deltaMaxX[item]);
    }


    // Increase width of inbetween edges
    for(GraphicsNodeItem * item: items)
    {
        InbetweenEdge * inbetweenEdge = item->node()->cell()->toInbetweenEdge();
        if(inbetweenEdge)
        {
            // get after items
            double lastRight = 0;
            double widthAfterItems = 0;
            GraphicsNodeItem * firstAfterItem = item->after();
            GraphicsNodeItem * afterItem = firstAfterItem;
            if(afterItem)
            {
                lastRight = afterItem->x() + 0.5*afterItem->width();
                widthAfterItems += afterItem->width();
                afterItem = afterItem->next();
            }
            while(afterItem && afterItem != firstAfterItem && afterItem->before() == item)
            {
                double right = afterItem->x() + 0.5*afterItem->width();
                if(right-lastRight < 0)
                    widthAfterItems += afterItem->width();
                else
                    widthAfterItems += right-lastRight;
                lastRight = right;
                afterItem = afterItem->next();
            }

            // get before items
            double lastLeft = 0;
            double widthBeforeItems = 0;
            GraphicsNodeItem * firstBeforeItem = item->before();
            GraphicsNodeItem * beforeItem = firstBeforeItem;
            if(beforeItem)
            {
                lastLeft = beforeItem->x() - 0.5*beforeItem->width();
                widthBeforeItems += beforeItem->width();
                beforeItem = beforeItem->previous();
            }
            while(beforeItem && beforeItem != firstBeforeItem && beforeItem->after() == item)
            {
                double left = beforeItem->x() - 0.5*beforeItem->width();
                if(lastLeft-left < 0)
                    widthBeforeItems += beforeItem->width();
                else
                    widthBeforeItems += lastLeft-left;
                lastLeft = left;
                beforeItem = beforeItem->previous();
            }

            // grow inbetween edge
            double widthBeforeAfter = std::max(widthBeforeItems,widthAfterItems);
            if(60 /*item->width()*/ < widthBeforeAfter)
            {
                item->setWidth(/*0.999 **/ widthBeforeAfter);
            }
        }
    }

    // After arrow contribution
    it = QMapIterator<GraphicsNodeItem*, GraphicsArrowItem*>(itemToAfterArrow_);
    while(it.hasNext())
    {
        it.next();
        GraphicsNodeItem * item = it.key();

        // Idea: for inbetween cells, keep double arrows vertical
        if(item->node()->cell()->toInbetweenCell())
        {
            GraphicsNodeItem * afterItem = item->after();
            GraphicsNodeItem * beforeAfterItem = afterItem->before();

            if(beforeAfterItem == item)
            {
                double delta = afterItem->pos().x()-0.5*afterItem->width() - (item->pos().x()-0.5*item->width());
                deltaX[item] += delta;
                deltaXNum[item] += 1;
                deltaX[afterItem] += -delta;
                deltaXNum[afterItem] += 1;
            }
        }
    }


    // Before arrow contribution
    it = QMapIterator<GraphicsNodeItem*, GraphicsArrowItem*>(itemToBeforeArrow_);
    while(it.hasNext())
    {
        it.next();
        GraphicsNodeItem * item = it.key();

        // Idea: for inbetween cells, keep double arrows vertical
        if(item->node()->cell()->toInbetweenCell())
        {
            GraphicsNodeItem * beforeItem = item->before();
            GraphicsNodeItem * afterBeforeItem = beforeItem->after();
            if(afterBeforeItem == item)
            {
                double delta = beforeItem->pos().x()+0.5*beforeItem->width() - (item->pos().x()+0.5*item->width());
                deltaX[item] += delta;
                deltaXNum[item] += 1;
                deltaX[beforeItem] += -delta;
                deltaXNum[beforeItem] += 1;
            }
        }
    }

    // Compute average of delta
    for(GraphicsNodeItem * item: items)
    {
        if(deltaMinX[item] > deltaMaxX[item])
        {
            deltaX[item] = 0.5 * (deltaMinX[item] + deltaMaxX[item]);
        }
        else
        {
            if(deltaXNum[item] > 0)
                deltaX[item] /= deltaXNum[item];

            deltaX[item] = std::max( deltaX[item] , deltaMinX[item] );
            deltaX[item] = std::min( deltaX[item] , deltaMaxX[item] );
        }
    }

    // Move nodes
    //double ratio = 0.99;
    QMapIterator<GraphicsNodeItem*, double> it3(deltaX);
    while(it3.hasNext())
    {
        it3.next();

        GraphicsNodeItem * item = it3.key();
        double delta = it3.value();
        double ratio = 0.8;//Random::random(0.8,0.95);
        if(!item->isMoved())
            item->moveBy(ratio*delta, 0);
    }

    // update arrows
    it = QMapIterator<GraphicsNodeItem*, GraphicsArrowItem*>(itemToNextArrow_);
    while(it.hasNext())
    {
        it.next();

        GraphicsNodeItem * item = it.key();
        GraphicsNodeItem * nextItem = item->next();
        GraphicsArrowItem * nextArrow = it.value();

        double y1 = item->pos().y();
        if(!nextItem->node()->cell()->toInbetweenEdge())
            y1 = nextItem->pos().y();
        double y2 = y1;
        double y1min = item->y() - 0.5*item->height()+10;
        double y1max = item->y() + 0.5*item->height()-10;
        double y2min = nextItem->y() - 0.5*nextItem->height()+10;
        double y2max = nextItem->y() + 0.5*nextItem->height()-10;
        if(y1 < y1min)
            y1 = y1min;
        if(y1 > y1max)
            y1 = y1max;
        if(y2 < y2min)
            y2 = y2min;
        if(y2 > y2max)
            y2 = y2max;

        QPointF startVec(item->x()+0.5*item->width(),y1);
        QPointF endVec(nextItem->x()-0.5*nextItem->width(),y2);

        nextArrow->setEndPoints(startVec,endVec);
    }
    it = QMapIterator<GraphicsNodeItem*, GraphicsArrowItem*>(itemToPreviousArrow_);
    while(it.hasNext())
    {
        it.next();

        GraphicsNodeItem * item = it.key();
        GraphicsNodeItem * previousItem = item->previous();
        GraphicsArrowItem * previousArrow = it.value();

        double y1 = item->pos().y();
        if(!previousItem->node()->cell()->toInbetweenEdge())
            y1 = previousItem->pos().y();
        double y2 = y1;
        double y1min = item->y() - 0.5*item->height()+10;
        double y1max = item->y() + 0.5*item->height()-10;
        double y2min = previousItem->y() - 0.5*previousItem->height()+10;
        double y2max = previousItem->y() + 0.5*previousItem->height()-10;
        if(y1 < y1min)
            y1 = y1min;
        if(y1 > y1max)
            y1 = y1max;
        if(y2 < y2min)
            y2 = y2min;
        if(y2 > y2max)
            y2 = y2max;

        QPointF startVec(item->x()-0.5*item->width(),y1);
        QPointF endVec(previousItem->x()+0.5*previousItem->width(),y2);

        previousArrow->setEndPoints(startVec,endVec);
    }
    it = QMapIterator<GraphicsNodeItem*, GraphicsArrowItem*>(itemToNextArrowBorder_);
    while(it.hasNext())
    {
        it.next();

        GraphicsNodeItem * item = it.key();
        GraphicsNodeItem * nextItem = item->next();
        GraphicsArrowItem * nextArrow = it.value();

        double y1 = item->pos().y();
        if(!nextItem->node()->cell()->toInbetweenEdge())
            y1 = nextItem->pos().y();
        double y2 = y1;
        double y1min = item->y() - 0.5*item->height()+10;
        double y1max = item->y() + 0.5*item->height()-10;
        double y2min = nextItem->y() - 0.5*nextItem->height()+10;
        double y2max = nextItem->y() + 0.5*nextItem->height()-10;
        if(y1 < y1min)
            y1 = y1min;
        if(y1 > y1max)
            y1 = y1max;
        if(y2 < y2min)
            y2 = y2min;
        if(y2 > y2max)
            y2 = y2max;

        QPointF startVec(item->x()+0.5*item->width(),y1);
        QPointF endVec(item->x()+0.5*item->width()+ARROW_LENGTH,y2);

        nextArrow->setEndPoints(startVec,endVec);
    }
    it = QMapIterator<GraphicsNodeItem*, GraphicsArrowItem*>(itemToPreviousArrowBorder_);
    while(it.hasNext())
    {
        it.next();

        GraphicsNodeItem * item = it.key();
        GraphicsNodeItem * previousItem = item->previous();
        GraphicsArrowItem * previousArrow = it.value();

        double y1 = item->pos().y();
        if(!previousItem->node()->cell()->toInbetweenEdge())
            y1 = previousItem->pos().y();
        double y2 = y1;
        double y1min = item->y() - 0.5*item->height()+10;
        double y1max = item->y() + 0.5*item->height()-10;
        double y2min = previousItem->y() - 0.5*previousItem->height()+10;
        double y2max = previousItem->y() + 0.5*previousItem->height()-10;
        if(y1 < y1min)
            y1 = y1min;
        if(y1 > y1max)
            y1 = y1max;
        if(y2 < y2min)
            y2 = y2min;
        if(y2 > y2max)
            y2 = y2max;

        QPointF startVec(item->x()-0.5*item->width(),y1);
        QPointF endVec(item->x()-0.5*item->width()-ARROW_LENGTH,y2);

        previousArrow->setEndPoints(startVec,endVec);
    }
    it = QMapIterator<GraphicsNodeItem*, GraphicsArrowItem*>(itemToAfterArrow_);
    while(it.hasNext())
    {
        it.next();

        GraphicsNodeItem * item = it.key();
        GraphicsNodeItem * afterItem = item->after();
        GraphicsArrowItem * afterArrow = it.value();

        double x1 = item->pos().x();
        if(!afterItem->node()->cell()->toInbetweenEdge())
            x1 = afterItem->pos().x();
        double x2 = x1;
        double x1min = item->x() - 0.5*item->width()+10;
        double x1max = item->x() + 0.5*item->width()-10;
        double x2min = afterItem->x() - 0.5*afterItem->width()+10;
        double x2max = afterItem->x() + 0.5*afterItem->width()-10;
        if(x1 < x1min)
            x1 = x1min;
        if(x1 > x1max)
            x1 = x1max;
        if(x2 < x2min)
            x2 = x2min;
        if(x2 > x2max)
            x2 = x2max;

        QPointF startVec(x1,item->y()+0.5*item->height());
        QPointF endVec(x2, afterItem->y() - 0.5*afterItem->height());

        afterArrow->setEndPoints(startVec,endVec);
    }
    it = QMapIterator<GraphicsNodeItem*, GraphicsArrowItem*>(itemToBeforeArrow_);
    while(it.hasNext())
    {
        it.next();

        GraphicsNodeItem * item = it.key();
        GraphicsNodeItem * beforeItem = item->before();
        GraphicsArrowItem * beforeArrow = it.value();

        double x1 = item->pos().x();
        if(!beforeItem->node()->cell()->toInbetweenEdge())
            x1 = beforeItem->pos().x();
        double x2 = x1;
        double x1min = item->x() - 0.5*item->width()+10;
        double x1max = item->x() + 0.5*item->width()-10;
        double x2min = beforeItem->x() - 0.5*beforeItem->width()+10;
        double x2max = beforeItem->x() + 0.5*beforeItem->width()-10;
        if(x1 < x1min)
            x1 = x1min;
        if(x1 > x1max)
            x1 = x1max;
        if(x2 < x2min)
            x2 = x2min;
        if(x2 > x2max)
            x2 = x2max;


        QPointF startVec(x1,item->y()-0.5*item->height());
        QPointF endVec(x2, beforeItem->y() + 0.5*beforeItem->height());

        beforeArrow->setEndPoints(startVec,endVec);
    }
}


