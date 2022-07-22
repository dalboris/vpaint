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

#ifndef ANIMATEDCYCLEWIDGET_H
#define ANIMATEDCYCLEWIDGET_H

#include <QWidget>
#include "VectorAnimationComplex/AnimatedCycle.h"
#include "VectorAnimationComplex/CellObserver.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QTimer>

using namespace VectorAnimationComplex;

enum SocketType {
    PreviousSocket = 0,
    NextSocket,
    BeforeSocket,
    AfterSocket
};

class GraphicsArrowItem;
class GraphicsSocketItem;
class AnimatedCycleWidget;
class GraphicsNodeItem: public QGraphicsPathItem, public CellObserver
{
public:
    // Enable the use of qgraphicsitem_cast<>
    enum { Type = UserType + 1 };
    int type() const { return Type; }

    GraphicsNodeItem(AnimatedCycleWidget * widget, Cell * cell, bool side = true);
    ~GraphicsNodeItem();
    void observedCellDeleted(Cell *);

    Cell * cell() const { return cell_; }
    bool side() const { return side_; }
    void setSide(bool b);
    bool isRoot() const { return isRoot_; } // whether this node is the root node
    void setRoot(bool b);
    bool isLeft() const { return isLeft_; } // whether this node is a "left node" (i.e., arrows wrap here)
    void setLeft(bool b);

    GraphicsNodeItem * next();
    GraphicsNodeItem * previous();
    GraphicsNodeItem * before();
    GraphicsNodeItem * after();

    GraphicsSocketItem * previousSocket() const { return sockets[PreviousSocket]; }
    GraphicsSocketItem * nextSocket() const { return sockets[NextSocket]; }
    GraphicsSocketItem * beforeSocket() const { return sockets[BeforeSocket]; }
    GraphicsSocketItem * afterSocket() const { return sockets[AfterSocket]; }

    QSet<GraphicsArrowItem*> backPointers() const { return backPointers_; }

    bool isMoved() const;

    int height() const;
    int width() const;
    int abstractWidth() const;
    QRectF rect() const;

    void setHeight(int i);
    void setWidth(int w);
    void setAbstractWidth(int w);
    void setFixedY(int y);

    void updateStyle();
    void updateText();
    void updateArrows();

    AnimatedCycleWidget * widget() const { return widget_; }

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent * event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);

private:
    friend class GraphicsArrowItem;
    friend class GraphicsSocketItem;
    void setPen_();
    void setPath_();
    void destruct_();

    Cell * cell_;
    bool side_;
    bool isRoot_;
    bool isLeft_;
    QGraphicsTextItem * text_;
    AnimatedCycleWidget * widget_;

    // We use integers to avoid floating point rounding errors
    // when computing the width of elements
    bool isMoved_;
    int width_;
    int awidth_; // abstract width
    int height_;
    int y_;
    Qt::MouseButton mouseButton_;

    GraphicsSocketItem* sockets[4];
    QSet<GraphicsArrowItem*> backPointers_;
    GraphicsArrowItem* arrowItem_;
};

class GraphicsSocketItem: public QGraphicsEllipseItem
{
public:
    // Enable the use of qgraphicsitem_cast<>
    enum { Type = UserType + 2 };
    int type() const override { return Type; }

    GraphicsSocketItem(SocketType socketType, GraphicsNodeItem * sourceItem);
    ~GraphicsSocketItem();

    GraphicsNodeItem * sourceItem() const { return sourceItem_ ; }
    GraphicsNodeItem * targetItem() const;
    SocketType socketType() const { return socketType_; }
    GraphicsArrowItem * arrowItem() const { return arrowItem_ ; }

    void setTargetItem(GraphicsNodeItem * target);

    void updatePosition();
    void updateStyle();

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent * event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent * event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent * event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent * event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent * event) override;

private:
    SocketType socketType_;
    GraphicsNodeItem* sourceItem_;
    GraphicsArrowItem* arrowItem_;
    bool isHovered_;

    Qt::MouseButton mouseButton_;

    bool isValid_();
    void setHighlighted_();
    void unsetHighlighted_();
};

class GraphicsArrowItem: public QGraphicsPathItem
{
public:
    // Enable the use of qgraphicsitem_cast<>
    enum { Type = UserType + 3 };
    int type() const { return Type; }

    // Creates an arrow starting at a socket
    GraphicsArrowItem(GraphicsSocketItem * socketItem);

    // Creates an arrow starting at a node.
    //
    // This is useful for temporary arrows created with right-click that do not
    // yet have a socket item (setting the socket item is deferred and
    // automatically computed on mouse move / mouse release).
    //
    GraphicsArrowItem(GraphicsNodeItem * sourceItem);

    // Destrow the GraphicsArrowItem
    ~GraphicsArrowItem();

    void setSocketItem(GraphicsSocketItem * socketItem);
    void setSourceItem(GraphicsNodeItem * sourceItem);
    void setTargetItem(GraphicsNodeItem * targetItem);
    void setEndPoint(const QPointF & p);

    GraphicsSocketItem * socketItem() const { return socketItem_; }
    GraphicsNodeItem * sourceItem() const { return sourceItem_; }
    GraphicsNodeItem * targetItem() const { return targetItem_; }

    QPointF endPoint() const { return endPoint_; }
    bool isBorderArrow() const;
    void updatePath();

private:
    GraphicsSocketItem * socketItem_;
    GraphicsNodeItem * sourceItem_;
    GraphicsNodeItem * targetItem_;
    QPointF endPoint_;

    void init_();
};

class AnimatedCycleWidget;
class AnimatedCycleGraphicsView: public QGraphicsView
{
public:
    AnimatedCycleGraphicsView(QGraphicsScene * scene);

protected:
    virtual void mousePressEvent(QMouseEvent * event);
    virtual void mouseMoveEvent(QMouseEvent * event);
    virtual void mouseReleaseEvent(QMouseEvent * event);
    virtual void wheelEvent(QWheelEvent *event);
};

class AnimatedCycleWidget : public QWidget, public CellObserver
{
    Q_OBJECT

public:
    explicit AnimatedCycleWidget(QWidget *parent = 0);
    ~AnimatedCycleWidget();

    // Set current animated cycle
    void setAnimatedCycle(const AnimatedCycle & animatedCycle);
    void setAnimatedCycle(InbetweenFace * inbetweenFace, int indexCycle);
    void clearAnimatedCycle();

    void observedCellDeleted(Cell *);

    // Convert graphics scene to animated cycle.
    // Notes: * The returned animated cycle is only aware of the nodes connected to first.
    //        * The returned animated cycle might be in an invalid state
    AnimatedCycle getAnimatedCycle() const;

    void start();
    void stop();

    void setReadOnly(bool b);
    bool isReadOnly() const;

    QList<GraphicsNodeItem*> nodeItems() const;

    void computeItemsWidth();
    void setRoot(GraphicsNodeItem * node);
    void updateLeftNodes(); // update the nodes where arrows should wrap

    void computeTimespan();
    Time beforeTime() const { return beforeTime_; }
    Time afterTime() const { return afterTime_; }

public slots:
    void reload();

private slots:    
    void apply();
    void animate();
    void addSelectedCells();
    void toggleHelp();

private:
    void clearScene();
    void createItem(Cell * cell);
    void computeItemsHeightAndY();
    void computeSceneFromAnimatedCycle(const AnimatedCycle & animatedCycle);

    QGraphicsScene * scene_;
    AnimatedCycleGraphicsView * view_;
    GraphicsNodeItem* root_;
    QTimer timer_;
    bool isReadOnly_;
    InbetweenFace * inbetweenFace_;
    int indexCycle_;
    QWidget * editModeExtras_;
    QWidget * help_;
    QPushButton * helpButton_;
    Time beforeTime_;
    Time afterTime_;
};

#endif // ANIMATEDCYCLEWIDGET_H
