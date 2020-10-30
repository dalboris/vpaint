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

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QVector2D>
#include <QGraphicsSceneMouseEvent>
#include <QMouseEvent>
#include <QPushButton>

#include "Global.h"
#include "MainWindow.h"
#include "Scene.h"

#include "Random.h"

namespace
{
const double ARROW_LENGTH = 30;
const double NODE_BORDER_RADIUS = 13;
const double NODE_SMALL_SIDE = 26;
const double NODE_LARGE_SIDE = 60;
const double SOCKET_RADIUS = 4;
}

GraphicsNodeItem::GraphicsNodeItem(AnimatedCycleWidget * widget, Cell * cell, bool side) :
    cell_(cell),
    side_(side),
    widget_(widget),
    isMoved_(false)
{
    // Set width, height, and color
    if(cell->toKeyVertex())
    {
        setBrush(QColor(255,170,170));
        width_ =  NODE_SMALL_SIDE;
        height_ = NODE_SMALL_SIDE;
    }
    else if(cell->toKeyEdge())
    {
        setBrush(QColor(170,204,255));
        width_ =  NODE_LARGE_SIDE;
        height_ = NODE_SMALL_SIDE;
    }
    else if(cell->toInbetweenVertex())
    {
        setBrush(QColor(255,218,218));
        width_ =  NODE_SMALL_SIDE;
        height_ = NODE_LARGE_SIDE;
    }
    else if(cell->toInbetweenEdge())
    {
        setBrush(QColor(235,243,255));
        width_ =  NODE_LARGE_SIDE;
        height_ = NODE_LARGE_SIDE;
    }

    // Create text label as a child item. Note: the text must be created before
    // the sockets to ensure that the sockets are correctly hoverable.
    text_ = new QGraphicsTextItem(QString(), this);
    text_->setFont(QFont("arial",7));
    updateText();

    // Create sockets as child items
    for (SocketType type : {PreviousSocket, NextSocket, BeforeSocket, AfterSocket}) {
        sockets[type] = new GraphicsSocketItem(type, this);
    }

    // Update path of this item and sockets
    setPath_();

    // Make item movable
    setFlag(ItemIsMovable, true);

    // Observe cell
    observe(cell);
}

GraphicsNodeItem::~GraphicsNodeItem()
{
    unobserve(cell_);
}

void GraphicsNodeItem::setSide(bool b)
{
    side_ = b;
    updateText();
}

void GraphicsNodeItem::observedCellDeleted(Cell *)
{
    widget()->reload();
}

void GraphicsNodeItem::updateText()
{
    QString string;
    string.setNum(cell()->id());
    if(cell()->toEdgeCell()) {
        string += side() ? "+" : "-";
    }

    text_->setPlainText(string);

    double textWidth = text_->boundingRect().width();
    double textHeight = text_->boundingRect().height();
    text_->setPos(-0.5*textWidth,-0.5*textHeight);
}

void GraphicsNodeItem::updateArrows()
{
    for (GraphicsSocketItem* socket : sockets) {
        if (socket->arrowItem()) {
            socket->arrowItem()->updatePath();
        }
    }
}

void GraphicsNodeItem::setPath_()
{
    QPainterPath path;
    EdgeCell * edge = cell()->toEdgeCell();
    QRectF r = rect();
    double s = NODE_BORDER_RADIUS;
    double hs = 0.5 * NODE_BORDER_RADIUS;
    if(edge && edge->isClosed())
    {
        path.moveTo(r.topLeft() + QPointF(-s, 0));
        path.lineTo(r.topRight() + QPointF(-s, 0));
        path.cubicTo(r.topRight() + QPointF(-hs, 0),
                     r.topRight() + QPointF(0, hs),
                     r.topRight() + QPointF(0, s));
        path.lineTo(r.bottomRight() + QPointF(0, -s));
        path.cubicTo(r.bottomRight() + QPointF(0, -hs),
                     r.bottomRight() + QPointF(-hs, 0),
                     r.bottomRight() + QPointF(-s, 0));
        path.lineTo(r.bottomLeft() + QPointF(-s, 0));
        path.cubicTo(r.bottomLeft() + QPointF(-hs, 0),
                     r.bottomLeft() + QPointF(0, -hs),
                     r.bottomLeft() + QPointF(0, -s));
        path.lineTo(r.topLeft() + QPointF(0, s));
        path.cubicTo(r.topLeft() + QPointF(0, hs),
                     r.topLeft() + QPointF(-hs, 0),
                     r.topLeft() + QPointF(-s, 0));
    }
    else
    {
        path.addRoundedRect(r, s, s);
    }
    setPath(path);
    for (GraphicsSocketItem* socket : sockets) {
        socket->updatePosition();
    }
}

double GraphicsNodeItem::width() const
{
    return width_;
}

double GraphicsNodeItem::height() const
{
    return height_;
}

QRectF GraphicsNodeItem::rect() const
{
    double hinting = 0.5; // ensures pixel-perfect strokes
    return QRectF(-0.5*width_ + hinting,
                  -0.5*height_ + hinting,
                  width_, height_);
}

void GraphicsNodeItem::setWidth(double w)
{
    width_ = w;
    setPath_();
}

void GraphicsNodeItem::setHeight(int i)
{
    height_ = i*NODE_LARGE_SIDE + (i-1)*(NODE_SMALL_SIDE+2*ARROW_LENGTH);
    setPath_();
}

void GraphicsNodeItem::setFixedY(double y)
{
    y_ = y;
    setY(y_);
}

void GraphicsNodeItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    if (event->button() == Qt::LeftButton) {
        if (!widget()->isReadOnly() && (event->modifiers() & Qt::CTRL)) {
            setSide(!side());
        }
        else {
            isMoved_ = true;
        }
    }
    QGraphicsPathItem::mousePressEvent(event);
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
    }
    QGraphicsPathItem::mouseReleaseEvent(event);
}

bool GraphicsNodeItem::isMoved() const
{
    return isMoved_;
}

GraphicsNodeItem * GraphicsNodeItem::next()
{
    return nextSocket()->targetItem();
}
GraphicsNodeItem * GraphicsNodeItem::previous()
{
    return previousSocket()->targetItem();
}
GraphicsNodeItem * GraphicsNodeItem::before()
{
    return beforeSocket()->targetItem();
}
GraphicsNodeItem * GraphicsNodeItem::after()
{
    return afterSocket()->targetItem();
}

GraphicsSocketItem::GraphicsSocketItem(SocketType socketType, GraphicsNodeItem * sourceItem) :
    QGraphicsEllipseItem(0, 0, 2*SOCKET_RADIUS, 2*SOCKET_RADIUS, sourceItem),
    socketType_(socketType),
    sourceItem_(sourceItem),
    arrowItem_(nullptr)
{
    setPen(QPen(Qt::black, 1.0));
    setBrush(QBrush(Qt::white));
    updatePosition();
    setAcceptHoverEvents(true);
}

GraphicsSocketItem::~GraphicsSocketItem()
{
    delete arrowItem_;
}

GraphicsNodeItem * GraphicsSocketItem::targetItem() const
{
    return arrowItem_ ? arrowItem_->targetItem() : nullptr;
}

void GraphicsSocketItem::setTargetItem(GraphicsNodeItem * target)
{
    if (arrowItem_) {
        arrowItem_->setTargetItem(target);
    }
    else {
        arrowItem_ = new GraphicsArrowItem(this);
        arrowItem_->setTargetItem(target);
        scene()->addItem(arrowItem_);
    }
    // TODO: what if target == nullptr?
    // TODO: how to decide if the arrow should be a border arrow?
}

void GraphicsSocketItem::updatePosition()
{
    double radius = SOCKET_RADIUS;
    QSizeF s(2*radius, 2*radius);
    QPointF hs(radius, radius);
    QRectF r = sourceItem_->rect();
    setRect(-radius, -radius, 2*radius, 2*radius);
    double offset = 20;
    switch (socketType()) {
    case PreviousSocket:
        if (offset + radius > 0.5 * r.height()) {
            offset = 0.5 * r.height() - radius;
        }
        setPos(r.bottomLeft() + QPointF(0, -offset));
        break;
    case NextSocket:
        if (offset + radius > 0.5 * r.height()) {
            offset = 0.5 * r.height() - radius;
        }
        setPos(r.topRight() + QPointF(0, offset));
        break;
    case BeforeSocket:
        if (offset + radius > 0.5 * r.width()) {
            offset = 0.5 * r.width() - radius;
        }
        setPos(r.topRight() + QPointF(-offset, 0));
        break;
    case AfterSocket:
        if (offset + radius > 0.5 * r.width()) {
            offset = 0.5 * r.width() - radius;
        }
        setPos(r.bottomLeft() + QPointF(offset, 0));
        break;
    }
}

void GraphicsSocketItem::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    if (!sourceItem()->widget()->isReadOnly()) {
        setBrush(QBrush(Qt::red));
        sourceItem()->setFlag(ItemIsMovable, false);
    }
}

void GraphicsSocketItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    if (!sourceItem()->widget()->isReadOnly()) {
        setBrush(QBrush(Qt::white));
        sourceItem()->setFlag(ItemIsMovable, true);
    }
}

void GraphicsSocketItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    if (!sourceItem()->widget()->isReadOnly()) {
        if (!arrowItem_) {
            arrowItem_ = new GraphicsArrowItem(this);
            scene()->addItem(arrowItem_);
        }
        arrowItem_->setTargetItem(nullptr);
        arrowItem_->setEndPoint(event->scenePos());
    }
}

void GraphicsSocketItem::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    if (!sourceItem()->widget()->isReadOnly()) {
        if (arrowItem_) {
            arrowItem_->setEndPoint(event->scenePos());
        }
    }
}

void GraphicsSocketItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
    if (!sourceItem()->widget()->isReadOnly()) {
        if (arrowItem_) {
            QGraphicsItem * item = scene()->itemAt(event->scenePos(), QTransform());
            GraphicsNodeItem * nodeItem = qgraphicsitem_cast<GraphicsNodeItem*>(item);
            if (!nodeItem) {
                QGraphicsTextItem * textItem = qgraphicsitem_cast<QGraphicsTextItem*>(item);
                if(textItem) {
                    nodeItem = qgraphicsitem_cast<GraphicsNodeItem*>(textItem->parentItem());
                }
            }
            if (nodeItem) {
                arrowItem_->setTargetItem(nodeItem);
            }
            else {
                delete arrowItem_;
                arrowItem_ = nullptr;
            }
        }
    }
}

GraphicsArrowItem::GraphicsArrowItem(GraphicsSocketItem * socketItem) :
    QGraphicsPathItem(),
    socketItem_(socketItem),
    targetItem_(nullptr),
    endPoint_(0.0, 0.0),
    isBorderArrow_(false)
{
    setPen(QPen(Qt::black, 1.0, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
    setBrush(QBrush(Qt::black));
}

void GraphicsArrowItem::setTargetItem(GraphicsNodeItem * target)
{
    targetItem_ = target;
    updatePath();
}

void GraphicsArrowItem::setEndPoint(const QPointF & p)
{
    endPoint_ = p;
    updatePath();
}

void GraphicsArrowItem::setIsBorderArrow(bool b)
{
    isBorderArrow_ = b;
    updatePath();
}

void GraphicsArrowItem::updatePath()
{
    // Get source point p1 (center of socket) and compute target point p2
    QPointF p1 = socketItem()->scenePos();
    SocketType socketType = socketItem()->socketType();
    GraphicsNodeItem * targetNodeItem = targetItem();
    GraphicsNodeItem * sourceNodeItem = socketItem()->sourceItem();
    QPointF p2;
    if (targetNodeItem) {
        // Get source and target rect in scene coordinates. This implementation
        // works because we don't use any scaling or rotation.
        QRectF targetRect = targetNodeItem->rect();
        targetRect.translate(targetNodeItem->pos());
        QRectF sourceRect = sourceNodeItem->rect();
        sourceRect.translate(sourceNodeItem->pos());
        // Compute target point p2
        double x1 = p1.x();
        double y1 = p1.y();
        double x2 = x1;
        double y2 = y1;
        double x2min = targetRect.left();
        double x2max = targetRect.right();
        double y2min = targetRect.top();
        double y2max = targetRect.bottom();
        if(x2 < x2min) x2 = x2min;
        if(x2 > x2max) x2 = x2max;
        if(y2 < y2min) y2 = y2min;
        if(y2 > y2max) y2 = y2max;

        if (socketType == NextSocket) {
            if (isBorderArrow()) {
                p2 = QPointF(x1 + ARROW_LENGTH, y2);
            }
            else {
                p2 = QPointF(x2min, y2);
            }
        }
        else if (socketType == PreviousSocket) {
            if (isBorderArrow()) {
                p2 = QPointF(x1 - ARROW_LENGTH, y2);
            }
            else {
                p2 = QPointF(x2max, y2);
            }
        }
        else if (socketType == BeforeSocket) {
            p2 = QPointF(x2, y2max);
        }
        else if (socketType == AfterSocket) {
            p2 = QPointF(x2, y2min);
        }
    }
    else {
        p2 = endPoint();
    }

    // Compute arrow geometry
    QVector2D n(p2 - p1);
    double length = n.length();
    if (length < SOCKET_RADIUS) {
        QPainterPath path;
        setPath(path);
    }
    else {
        QPointF u = (n / length).toPointF();
        QPointF v(-u.y(), u.x());
        double arrowHeadHalfWidth = 2;
        double arrowHeadLength = 4;
        double arrowEndMargin = 1.5;
        QPointF arrowStart = p1 + SOCKET_RADIUS * u;
        QPointF arrowEnd = p2 - arrowEndMargin * u;
        QPointF arrowHeadBase = arrowEnd - arrowHeadLength * u;
        QPointF arrowHeadOffset = arrowHeadHalfWidth * v;
        QPainterPath path;
        path.moveTo(arrowStart);
        path.lineTo(arrowHeadBase);
        path.moveTo(arrowEnd);
        path.lineTo(arrowHeadBase + arrowHeadOffset);
        path.lineTo(arrowHeadBase - arrowHeadOffset);
        path.closeSubpath();
        setPath(path);
    }
}

AnimatedCycleGraphicsView::AnimatedCycleGraphicsView(QGraphicsScene * scene) :
    QGraphicsView(scene)
{
    //setTransformationAnchor(AnchorUnderMouse);
}

void AnimatedCycleGraphicsView::wheelEvent(QWheelEvent *event)
{
    setTransformationAnchor(AnchorUnderMouse);
    double ratio = 1.0 / pow( 0.8f, (double) event->delta() / (double) 120.0f);
    scale(ratio, ratio);
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
    /*
    else if(event->button() == Qt::LeftButton && global()->keyboardModifiers().testFlag(Qt::AltModifier))
    {
        GraphicsNodeItem * nodeItem = nodeItemAt(event->pos());
        if(nodeItem)
        {
            animatedCycleWidget_->deleteItem(nodeItem);
            animatedCycleWidget_->save();
        }
    }
    */
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
    view_ = new AnimatedCycleGraphicsView(scene_);
    view_->setRenderHints(QPainter::Antialiasing);

    QPushButton * addSelectedCellsButton = new QPushButton("Add selected cells");
    QPushButton * reloadButton = new QPushButton("Reload");
    QPushButton * applyButton = new QPushButton("Apply");
    connect(addSelectedCellsButton, SIGNAL(clicked()), this, SLOT(addSelectedCells()));
    connect(reloadButton, SIGNAL(clicked()), this, SLOT(reload()));
    connect(applyButton, SIGNAL(clicked()), this, SLOT(apply()));

    QWidget * editorButtons = new QWidget();
    QHBoxLayout * editorButtonsLayout = new QHBoxLayout();
    editorButtonsLayout->addWidget(addSelectedCellsButton);
    editorButtonsLayout->addWidget(reloadButton);
    editorButtonsLayout->addWidget(applyButton);
    editorButtons->setLayout(editorButtonsLayout);

    QString ctrl = QString(ACTION_MODIFIER_NAME_SHORT).toUpper();
    editModeExtras_ = new QWidget();
    QVBoxLayout * editModeExtrasLayout = new QVBoxLayout();
    editModeExtrasLayout->addWidget(new QLabel(ctrl + " + Click: Toggle edge direction"));
    editModeExtrasLayout->addWidget(editorButtons);
    editModeExtras_->setLayout(editModeExtrasLayout);

    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget(view_);
    layout->addWidget(editModeExtras_);
    setLayout(layout);

    timer_.setInterval(16);
    connect(&timer_, SIGNAL(timeout()), this, SLOT(animate()));
}

void AnimatedCycleWidget::createItem(Cell * cell)
{
    GraphicsNodeItem * item = new GraphicsNodeItem(this, cell);
    scene_->addItem(item);
}

void AnimatedCycleWidget::addSelectedCells()
{
    VectorAnimationComplex::VAC * vac = global()->mainWindow()->scene()->activeVAC();
    if (vac)
    {
        CellSet selectedCells = vac->selectedCells();
        foreach(Cell * cell, selectedCells)
            createItem(cell);
        computeItemHeightAndY();
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
    if (isReadOnly_) {
        editModeExtras_->hide();
    }
    else {
        editModeExtras_->show();
    }
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
}

void AnimatedCycleWidget::observedCellDeleted(Cell *)
{
    clearAnimatedCycle();
}

void AnimatedCycleWidget::clearAnimatedCycle()
{
    if(inbetweenFace_)
    {
        unobserve(inbetweenFace_);
        inbetweenFace_ = 0;
        indexCycle_ = 0;
    }
    clearScene();
}

void AnimatedCycleWidget::setAnimatedCycle(InbetweenFace * inbetweenFace, int indexCycle)
{
    clearAnimatedCycle();
    if(inbetweenFace && indexCycle >= 0 && indexCycle < inbetweenFace->numAnimatedCycles())
    {
        inbetweenFace_ = inbetweenFace;
        indexCycle_ = indexCycle;
        observe(inbetweenFace_);
        reload();
    }
}

void AnimatedCycleWidget::setAnimatedCycle(const AnimatedCycle & animatedCycle)
{
    clearAnimatedCycle();
    computeSceneFromAnimatedCycle(animatedCycle);
}

void AnimatedCycleWidget::reload()
{
    clearScene();
    if(inbetweenFace_ && indexCycle_ >= 0 && indexCycle_ < inbetweenFace_->numAnimatedCycles())
    {
        AnimatedCycle animatedCycle = inbetweenFace_->animatedCycle(indexCycle_);
        computeSceneFromAnimatedCycle(animatedCycle);
        start();
    }
}

void AnimatedCycleWidget::apply()
{
    if(inbetweenFace_ && indexCycle_ >= 0 && indexCycle_ < inbetweenFace_->numAnimatedCycles())
    {
        inbetweenFace_->setCycle(indexCycle_, getAnimatedCycle());

        VectorAnimationComplex::VAC * vac = inbetweenFace_->vac();
        emit vac->needUpdatePicking();
        emit vac->changed();
        emit vac->checkpoint();
    }
}

AnimatedCycle AnimatedCycleWidget::getAnimatedCycle() const
{
    // Create all nodes from items
    QMap<GraphicsNodeItem*, AnimatedCycleNode*> itemToNode;
    QList<GraphicsNodeItem*> items = nodeItems();
    foreach (GraphicsNodeItem * item, items) {
        AnimatedCycleNode * node = new AnimatedCycleNode(item->cell());
        node->setSide(item->side());
        itemToNode[item] = node;
    }

    // Set connections betweens nodes
    foreach (GraphicsNodeItem * item, items) {
        AnimatedCycleNode * node = itemToNode[item];
        if (item->next()) {
            node->setNext(itemToNode[item->next()]);
        }
        if (item->previous()) {
            node->setPrevious(itemToNode[item->previous()]);
        }
        if (item->before()) {
            node->setBefore(itemToNode[item->before()]);
        }
        if (item->after()) {
            node->setAfter(itemToNode[item->after()]);
        }
    }

    // Find first node
    AnimatedCycleNode* first = nullptr;
    if (!items.isEmpty()) {
        GraphicsNodeItem* firstItem = items.first();
        int n = items.size(); // avoid infinite loop in case of invalid cycle
        int i = 0;
        while (firstItem->before() && i < n) {
            ++i;
            firstItem = firstItem->before();
        }
        first = itemToNode[firstItem];
    }

    // Create animated cycle
    AnimatedCycle res(first);

    // Delete unreachable nodes that would otherwise be leaked
    QSet<AnimatedCycleNode*> nodes = res.nodes();
    foreach (AnimatedCycleNode* node, itemToNode) {
        if (!nodes.contains(node)) {
            delete node;
        }
    }

    return res;
}

void AnimatedCycleWidget::deleteItem(GraphicsNodeItem * item)
{
    /*
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
    foreach(GraphicsArrowItem * arrowItem, arrowItems)
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
    */
}

QList<GraphicsNodeItem*> AnimatedCycleWidget::nodeItems() const
{
    QList<GraphicsNodeItem*> res;
    foreach(QGraphicsItem * i, scene_->items())
    {
        GraphicsNodeItem * item = qgraphicsitem_cast<GraphicsNodeItem *>(i);
        if(item)
            res << item;
    }
    return res;
}

void AnimatedCycleWidget::computeSceneFromAnimatedCycle(const AnimatedCycle & animatedCycle)
{
    // Clear scene
    clearScene();

    // Get start nodes
    QSet<AnimatedCycleNode*> startNodes;
    AnimatedCycleNode * startNode = animatedCycle.first();
    while(startNode)
    {
        startNodes << startNode;
        startNode = startNode->after();
    }

    // Create items
    QMap<AnimatedCycleNode*, GraphicsNodeItem*> nodeToItem;
    foreach(AnimatedCycleNode * node, animatedCycle.nodes())
    {
        GraphicsNodeItem * item = new GraphicsNodeItem(this, node->cell(), node->side());
        scene_->addItem(item);
        nodeToItem[node] = item;
    }

    // Set item height and Y
    computeItemHeightAndY();

    // Create arrows
    foreach(AnimatedCycleNode * node, animatedCycle.nodes())
    {
        GraphicsNodeItem * item = nodeToItem[node];
        if(node->next()) // can be false if cycle is invalid
        {
            GraphicsNodeItem * target = nodeToItem[node->next()];
            item->nextSocket()->setTargetItem(target);
            if(startNodes.contains(node->next())) {
                item->nextSocket()->arrowItem()->setIsBorderArrow(true);
            }
        }
        if(node->previous()) // can be false if cycle is invalid
        {
            GraphicsNodeItem * target = nodeToItem[node->previous()];
            item->previousSocket()->setTargetItem(target);
            if(startNodes.contains(node)) {
                item->previousSocket()->arrowItem()->setIsBorderArrow(true);
            }
        }
        if(node->after())
        {
            GraphicsNodeItem * target = nodeToItem[node->after()];
            item->afterSocket()->setTargetItem(target);
        }
        if(node->before())
        {
            GraphicsNodeItem * target = nodeToItem[node->before()];
            item->beforeSocket()->setTargetItem(target);
        }
    }
}

void AnimatedCycleWidget::computeItemHeightAndY()
{
    // Get key times
    QSet<int> keyTimes;
    QList<GraphicsNodeItem*> items = nodeItems();
    foreach(GraphicsNodeItem * item, items)
    {
        Cell * cell = item->cell();
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
    foreach(GraphicsNodeItem * item, items)
    {
        Cell * cell = item->cell();
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

            double yBefore = idBefore * (NODE_LARGE_SIDE + NODE_SMALL_SIDE + 2*ARROW_LENGTH);
            double yAfter = idAfter * (NODE_LARGE_SIDE + NODE_SMALL_SIDE + 2*ARROW_LENGTH);

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

            item->setFixedY(id * (NODE_LARGE_SIDE + NODE_SMALL_SIDE + 2*ARROW_LENGTH));
        }
    }
}

void AnimatedCycleWidget::animate()
{
    // Compute delta between current and target
    QMap<GraphicsNodeItem*, double> deltaX;
    QMap<GraphicsNodeItem*, int> deltaXNum;
    QMap<GraphicsNodeItem*, double> deltaMinX;
    QMap<GraphicsNodeItem*, double> deltaMaxX;

    // Initialize values
    QList<GraphicsNodeItem*> items = nodeItems();
    foreach(GraphicsNodeItem * item, items)
    {
        deltaX[item] = 0;
        deltaXNum[item] = 0;
        deltaMinX[item] = -10000;// std::numeric_limits<double>::lowest();
        deltaMaxX[item] = 10000;// std::numeric_limits<double>::max();
    }

    // Next arrow contribution
    foreach(GraphicsNodeItem * item, items)
    {
        if (item->next() && !item->nextSocket()->arrowItem()->isBorderArrow())
        {
            GraphicsNodeItem * nextItem = item->next();

            double start = item->pos().x() + 0.5*item->width();
            double end = nextItem->pos().x() - 0.5*nextItem->width();
            double vec = end - start;
            double delta = ARROW_LENGTH - vec;

            deltaMinX[nextItem] = std::max(delta,deltaMinX[nextItem]);
            deltaMaxX[item] = std::min(-delta,deltaMaxX[item]);
        }
    }

    // Previous arrow contribution    
    foreach(GraphicsNodeItem * nextItem, items)
    {
        if (nextItem->previous() && !nextItem->previousSocket()->arrowItem()->isBorderArrow())
        {
            GraphicsNodeItem * item = nextItem->previous();

            double start = item->pos().x() + 0.5*item->width();
            double end = nextItem->pos().x() - 0.5*nextItem->width();
            double vec = end - start;
            double delta = ARROW_LENGTH - vec;

            deltaMinX[nextItem] = std::max(delta,deltaMinX[nextItem]);
            deltaMaxX[item] = std::min(-delta,deltaMaxX[item]);
        }
    }

    // Increase width of inbetween edges
    foreach(GraphicsNodeItem * item, items)
    {
        InbetweenEdge * inbetweenEdge = item->cell()->toInbetweenEdge();
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
            if(NODE_LARGE_SIDE < widthBeforeAfter)
            {
                item->setWidth(widthBeforeAfter);
            }
        }
    }

    // After arrow contribution
    foreach(GraphicsNodeItem * item, items)
    {
        if (item->after())
        {
            // Idea: for inbetween cells, keep double arrows vertical
            if(item->cell()->toInbetweenCell())
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
    }

    // Before arrow contribution
    foreach(GraphicsNodeItem * item, items)
    {
        if (item->before())
        {
            // Idea: for inbetween cells, keep double arrows vertical
            if(item->cell()->toInbetweenCell())
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
    }

    // Compute average of delta
    foreach(GraphicsNodeItem * item, items)
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
    foreach(GraphicsNodeItem * item, items)
    {
        item->updateArrows();
    }
}
