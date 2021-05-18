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

#include <QStack>
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
// We use integers to avoid floating point rounding errors
// when computing the width of elements
const int ARROW_LENGTH = 30;
const int NODE_BORDER_RADIUS = 13;
const int NODE_SMALL_SIDE = 26;
const int NODE_LARGE_SIDE = 60;
const int SOCKET_RADIUS = 4;
}

GraphicsNodeItem::GraphicsNodeItem(AnimatedCycleWidget * widget, Cell * cell, bool side) :
    cell_(cell),
    side_(side),
    isRoot_(false),
    isLeft_(false),
    widget_(widget),
    isMoved_(false)
{
    // Set pen
    setPen_();

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
    while (!backPointers_.isEmpty()) {
        GraphicsArrowItem* arrow = *backPointers_.begin();
        arrow->setTargetItem(nullptr);
        // Note: the above line also removes the arrow from backPointers_
    }
}

void GraphicsNodeItem::setSide(bool b)
{
    side_ = b;
    updateText();
}

void GraphicsNodeItem::setRoot(bool b)
{
    isRoot_ = b;
    setPen_();
}

void GraphicsNodeItem::setLeft(bool b)
{
    isLeft_ = b;
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

void GraphicsNodeItem::setPen_()
{
    if (isRoot_) {
        setPen(QPen(Qt::black, 3.0));
    }
    else {
        setPen(QPen(Qt::black, 1.0));
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

int GraphicsNodeItem::width() const
{
    return width_;
}

int GraphicsNodeItem::abstractWidth() const
{
    return awidth_;
}

int GraphicsNodeItem::height() const
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

void GraphicsNodeItem::setWidth(int w)
{
    width_ = w;
    setPath_();
}

void GraphicsNodeItem::setAbstractWidth(int w)
{
    awidth_ = w;
    if (cell()->toEdgeCell())
    {
        setWidth(awidth_);
    }
}

void GraphicsNodeItem::setHeight(int i)
{
    height_ = i*NODE_LARGE_SIDE + (i-1)*(NODE_SMALL_SIDE+2*ARROW_LENGTH);
    setPath_();
}

void GraphicsNodeItem::setFixedY(int y)
{
    y_ = y;
    setY(y_);
}

void GraphicsNodeItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
    if (event->button() == Qt::LeftButton) {
        if (!widget()->isReadOnly() && (event->modifiers() == Qt::CTRL)) {
            setSide(!side());
        }
        else if (!widget()->isReadOnly() && (event->modifiers() == Qt::ALT)) {
            destruct_();
        }
        else if (!widget()->isReadOnly() && (event->modifiers() == Qt::SHIFT)) {
            widget()->setRoot(this);
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

void GraphicsNodeItem::destruct_()
{
    // Delete arrows pointing to this node item
    QList<GraphicsNodeItem*> items = widget()->nodeItems();
    foreach (GraphicsNodeItem * item, items) {
        if (item->next() == this) {
            item->nextSocket()->setTargetItem(nullptr);
        }
        if (item->previous() == this) {
            item->previousSocket()->setTargetItem(nullptr);
        }
        if (item->before() == this) {
            item->beforeSocket()->setTargetItem(nullptr);
        }
        if (item->after() == this) {
            item->afterSocket()->setTargetItem(nullptr);
        }
    }

    if (isRoot_) {
        widget()->setRoot(nullptr);
    }

    // Delete this item and all its chilren (socket+text).
    // Note that deleting each socket also deletes its arrow if any.
    delete this;
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
        if (target) {
            arrowItem_->setTargetItem(target);
        }
        else {
            scene()->removeItem(arrowItem_);
            delete arrowItem_;
            arrowItem_ = nullptr;
        }
    }
    else if (target) {
        arrowItem_ = new GraphicsArrowItem(this);
        arrowItem_->setTargetItem(target);
        scene()->addItem(arrowItem_);
    }
    sourceItem()->widget()->updateLeftNodes();
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
                arrowItem_->setTargetItem(nullptr);
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
    endPoint_(0.0, 0.0)
{
    setPen(QPen(Qt::black, 1.0, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
    setBrush(QBrush(Qt::black));
}

GraphicsArrowItem::~GraphicsArrowItem()
{
    if (targetItem_) {
        targetItem_->backPointers_.remove(this);
    }
}

void GraphicsArrowItem::setTargetItem(GraphicsNodeItem * target)
{
    if (targetItem_) {
        targetItem_->backPointers_.remove(this);
    }
    targetItem_ = target;
    if (targetItem_) {
        targetItem_->backPointers_.insert(this);
    }
    sourceItem()->widget()->computeItemsWidth();
    updatePath();
}

void GraphicsArrowItem::setEndPoint(const QPointF & p)
{
    endPoint_ = p;
    updatePath();
}

bool GraphicsArrowItem::isBorderArrow() const
{
    return targetItem() && (
               (socketItem()->socketType() == NextSocket     && targetItem()->isLeft()) ||
               (socketItem()->socketType() == PreviousSocket && sourceItem()->isLeft())
           );
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
            if (isBorderArrow() && x2min < x1) {
                p2 = QPointF(x1 + ARROW_LENGTH, y2);
            }
            else {
                p2 = QPointF(x2min, y2);
            }
        }
        else if (socketType == PreviousSocket) {
            if (isBorderArrow() && x2max > x1) {
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
    root_(nullptr),
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
    editModeExtrasLayout->addWidget(new QLabel(ctrl + tr(" + Click: Toggle edge direction")));
    editModeExtrasLayout->addWidget(new QLabel(tr("ALT + Click: Delete node")));
    editModeExtrasLayout->addWidget(new QLabel(tr("SHIFT + Click: Change root node")));
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
        computeItemsHeightAndY();
        computeItemsWidth();
        if (!root_) {
            // Select a new root
            setRoot(nullptr);
        }
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
    root_ = nullptr;
    view_->setScene(nullptr);
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
        inbetweenFace_ = nullptr;
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

    // Create animated cycle
    AnimatedCycleNode * root = root_ ? itemToNode[root_] : nullptr;
    AnimatedCycle res(root);

    // Delete unreachable nodes that would otherwise be leaked
    QSet<AnimatedCycleNode*> nodes = res.nodes();
    foreach (AnimatedCycleNode* node, itemToNode) {
        if (!nodes.contains(node)) {
            delete node;
        }
    }

    return res;
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

    // Create items
    AnimatedCycleNode * root = animatedCycle.root();
    QSet<AnimatedCycleNode*> nodes = animatedCycle.nodes();
    QMap<AnimatedCycleNode*, GraphicsNodeItem*> nodeToItem;
    foreach(AnimatedCycleNode * node, nodes)
    {
        GraphicsNodeItem * item = new GraphicsNodeItem(this, node->cell(), node->side());
        if (node == root) {
            item->setRoot(true);
            root_ = item;
        }
        scene_->addItem(item);
        nodeToItem[node] = item;
    }

    // Set item height and Y
    computeItemsHeightAndY();

    // Create arrows
    foreach(AnimatedCycleNode * node, nodes)
    {
        GraphicsNodeItem * item = nodeToItem[node];
        if(node->next()) // can be false if cycle is invalid
        {
            GraphicsNodeItem * target = nodeToItem[node->next()];
            item->nextSocket()->setTargetItem(target);
        }
        if(node->previous()) // can be false if cycle is invalid
        {
            GraphicsNodeItem * target = nodeToItem[node->previous()];
            item->previousSocket()->setTargetItem(target);
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

    updateLeftNodes();
    computeItemsWidth();
}

void AnimatedCycleWidget::computeItemsHeightAndY()
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

namespace {

// Abstract width computed by adding the abstract width of nodes
// that points to the given node via the given socket type.
int computeAdjustedAbstractWidth(GraphicsNodeItem * node, SocketType socketType)
{
    int adjustedAbstractWidth = - ARROW_LENGTH;
    for (GraphicsArrowItem * arrow : node->backPointers()) {
        GraphicsSocketItem * socket = arrow->socketItem();
        if (socket->socketType() == socketType) {
            GraphicsNodeItem * source = socket->sourceItem();
            if (source->abstractWidth() != -1) {
                InbetweenEdge * ie = node->cell()->toInbetweenEdge();
                if (!ie || !ie->isClosed()) {
                    adjustedAbstractWidth += ARROW_LENGTH + source->abstractWidth();
                }
            }
        }
    }
    return adjustedAbstractWidth;
}

// Abstract width computed by adding the abstract width of nodes
// that points to the given node via either the before or after socket type.
int computeAdjustedAbstractWidth(GraphicsNodeItem * node)
{
    int wb = computeAdjustedAbstractWidth(node, AfterSocket);
    int wa = computeAdjustedAbstractWidth(node, BeforeSocket);
    return std::max(wb, wa);
}

// Compute the top or bottom abstract width of the given inbetween open edge.
int computeAbstractWidth(GraphicsNodeItem * node, const Path& path, SocketType socketType)
{
    if (path.type() == Path::SingleVertex) {
        return NODE_LARGE_SIDE;
    }
    else if (path.type() == Path::OpenHalfedgeList) {
        int k = path.size();
        int defaultAbstractWidth = NODE_LARGE_SIDE +
                (k-1)*(NODE_SMALL_SIDE + NODE_LARGE_SIDE + 2*ARROW_LENGTH);
        int adjustedAbstractWidth = computeAdjustedAbstractWidth(node, socketType);
        return std::max(defaultAbstractWidth, adjustedAbstractWidth);
    }
    else { // Invalid
        return NODE_LARGE_SIDE;
    }
}

// Compute the top or bottom abstract width of the given inbetween closed edge.
int computeAbstractWidth(GraphicsNodeItem * node, const Cycle& cycle, SocketType socketType)
{
    if (cycle.type() == Cycle::SingleVertex) {
        return NODE_LARGE_SIDE;
    }
    else if (cycle.type() == Cycle::OpenHalfedgeList){
        int k = cycle.size();
        int defaultAbstractWidth = NODE_SMALL_SIDE + ARROW_LENGTH + NODE_LARGE_SIDE +
                (k-1)*(NODE_SMALL_SIDE + NODE_LARGE_SIDE + 2*ARROW_LENGTH);
        int adjustedAbstractWidth = computeAdjustedAbstractWidth(node, socketType);
        return std::max(defaultAbstractWidth, adjustedAbstractWidth);
    }
    else if (cycle.type() == Cycle::ClosedHalfedge){
        int k = cycle.size();
        int defaultAbstractWidth = NODE_LARGE_SIDE +
                (k-1)*(NODE_LARGE_SIDE + ARROW_LENGTH);
        int adjustedAbstractWidth = computeAdjustedAbstractWidth(node, socketType);
        return std::max(defaultAbstractWidth, adjustedAbstractWidth);
    }
    else { // Invalid
        return NODE_LARGE_SIDE;
    }
}

int computeAbstractWidth(GraphicsNodeItem * node)
{
    if (node->cell()->toKeyVertex()) {
        int defaultAbstractWidth = NODE_SMALL_SIDE;
        int adjustedAbstract = computeAdjustedAbstractWidth(node);
        return std::max(defaultAbstractWidth, adjustedAbstract);
    }
    if (node->cell()->toInbetweenVertex()) {
        int defaultAbstractWidth = NODE_SMALL_SIDE;
        return defaultAbstractWidth;
    }
    else if (node->cell()->toKeyEdge()) {
        int defaultAbstractWidth = NODE_LARGE_SIDE;
        return defaultAbstractWidth;
    }
    else if (InbetweenEdge* ie = node->cell()->toInbetweenEdge()) {
        if (ie->isClosed()) {
            Cycle cb = ie->beforeCycle();
            Cycle ca = ie->afterCycle();
            int wb = computeAbstractWidth(node, cb, AfterSocket);
            int wa = computeAbstractWidth(node, ca, BeforeSocket);
            return std::max(wb, wa);
        }
        else {
            Path pb = ie->beforePath();
            Path pa = ie->afterPath();
            int wb = computeAbstractWidth(node, pb, AfterSocket);
            int wa = computeAbstractWidth(node, pa, BeforeSocket);
            return std::max(wb, wa);
        }
    }
    return -1; // normally unreachable
}

}

void AnimatedCycleWidget::computeItemsWidth()
{
    // Get all node items
    QList<GraphicsNodeItem*> items = nodeItems();
    if (items.isEmpty()) {
        return;
    }

    // Allocate once a multi-purpose stack
    QStack<GraphicsNodeItem*> stack;
    stack.reserve(items.size());

    // Compute connected components
    int numConnectedComponents = 0;
    QMap<GraphicsNodeItem*, int> connectedComponents;
    QList<int> connectedComponentSize;
    foreach(GraphicsNodeItem * item, items) {
        connectedComponents[item] = -1;
    }
    foreach(GraphicsNodeItem * item, items) {
        if (connectedComponents[item] == -1) {
            int i = numConnectedComponents;
            int size = 0;
            ++numConnectedComponents;
            connectedComponents[item] = i;
            ++size;
            stack.clear();
            stack.push(item);
            while (!stack.isEmpty()) {
                GraphicsNodeItem * n = stack.pop();
                for (GraphicsNodeItem * m : {n->next(), n->previous(), n->before(), n->after()}) {
                    if (m && connectedComponents[m] == -1) {
                        connectedComponents[m] = i;
                        ++size;
                        stack.push(m);
                    }
                }
                for (GraphicsArrowItem * arrow : n->backPointers()) {
                    GraphicsNodeItem * m = arrow->sourceItem();
                    if (m && connectedComponents[m] == -1) {
                        connectedComponents[m] = i;
                        ++size;
                        stack.push(m);
                    }
                }
            }
            connectedComponentSize.push_back(size);
        }
    }

    // Compute abstract width of all nodes.
    //
    // The "abstract width" is like its actual visible width, except for key/inbetween vertices, where
    // their actual width is always set to NODE_SMALL_SIDE, but their abstract width could be wider
    // in case an edge shrinks to a vertex, in which case the abstract width of the vertex is the same
    // as the edge.
    //
    foreach(GraphicsNodeItem * item, items) {
        item->setAbstractWidth(-1);
    }
    foreach(GraphicsNodeItem * item, items) {
        if (item->abstractWidth() == -1) {
            stack.clear();
            stack.push(item);
            int connectedComponent = connectedComponents[item];
            int size = connectedComponentSize[connectedComponent];
            int maxIter = size * size;
            int numIter = 0;
            while (!stack.isEmpty() && numIter < maxIter) {
                ++numIter;
                GraphicsNodeItem * node = stack.pop();
                int oldAbstractWidth = node->abstractWidth();
                int newAbstractWidth = std::max(oldAbstractWidth, computeAbstractWidth(node));
                if (oldAbstractWidth != newAbstractWidth) {
                    node->setAbstractWidth(newAbstractWidth);
                    for (GraphicsNodeItem * m : {node->next(), node->previous(), node->before(), node->after()}) {
                        if (m) {
                            stack.push(m);
                        }
                    }
                    for (GraphicsArrowItem * arrow : node->backPointers()) {
                        GraphicsNodeItem * m = arrow->sourceItem();
                        if (m) {
                            stack.push(m);
                        }
                    }
                }
            }
        }
    }
}

void AnimatedCycleWidget::setRoot(GraphicsNodeItem * node)
{
    // If the given node is nullptr, we arbitrarily find a root
    if (!node) {
        foreach(QGraphicsItem * i, scene_->items()) {
            GraphicsNodeItem * item = qgraphicsitem_cast<GraphicsNodeItem *>(i);
            if(item) {
                node = item;
                break;
            }
        }
    }

    // Perform the change of root
    if (node != root_) {
        // Unset current root
        if (root_) {
            root_->setRoot(false);
            root_ = nullptr;
        }
        // Set current root
        if (node) {
            root_ = node;
            root_->setRoot(true);
        }

        updateLeftNodes();
    }
}

namespace {

GraphicsNodeItem * leftMostBefore(GraphicsNodeItem * item)
{
    if (item)
    {
        GraphicsNodeItem * before = item->before();
        if (before)
        {
            GraphicsNodeItem * leftMostBefore = before;
            GraphicsNodeItem * leftMostBeforeCandidate = leftMostBefore->previous();
            while (leftMostBeforeCandidate &&
                   leftMostBeforeCandidate != before &&
                   leftMostBeforeCandidate->after() == item)
            {
                leftMostBefore = leftMostBeforeCandidate;
                leftMostBeforeCandidate = leftMostBefore->previous();
            }
            return leftMostBefore;
        }
    }
    return nullptr;
}

} // namespace

void AnimatedCycleWidget::updateLeftNodes()
{
    QList<GraphicsNodeItem*> items = nodeItems();
    foreach(GraphicsNodeItem * item, items) {
        item->setLeft(false);
    }

    if (root_)
    {
        root_->setLeft(true);
        GraphicsNodeItem * item = leftMostBefore(root_);
        while(item)
        {
            item->setLeft(true);
            item = leftMostBefore(item);
        }
        item = root_->after();
        while(item)
        {
            item->setLeft(true);
            item = item->after();
        }
    }
}

void AnimatedCycleWidget::animate()
{
    // Get all node items
    QList<GraphicsNodeItem*> items = nodeItems();
    if (items.isEmpty()) {
        return;
    }

    // Compute delta between current and target
    QMap<GraphicsNodeItem*, double> deltaX;
    QMap<GraphicsNodeItem*, int> deltaXNum;
    QMap<GraphicsNodeItem*, double> deltaMinX;
    QMap<GraphicsNodeItem*, double> deltaMaxX;

    // Initialize values
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
