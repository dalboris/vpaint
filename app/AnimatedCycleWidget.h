// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef ANIMATEDCYCLEWIDGET_H
#define ANIMATEDCYCLEWIDGET_H

#include <QWidget>
#include "VectorAnimationComplex/AnimatedCycle.h"
#include "VectorAnimationComplex/CellObserver.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPathItem>
#include <QGraphicsPolygonItem>
#include <QTimer>
#include <QHBoxLayout>

using namespace VectorAnimationComplex;

class AnimatedCycleWidget;
class GraphicsNodeItem: public QGraphicsPathItem, public CellObserver
{
public:
    // Enable the use of qgraphicsitem_cast<>
    enum { Type = UserType + 1 };
    int type() const { return Type; }

    GraphicsNodeItem(AnimatedCycleNode * node, AnimatedCycleWidget * widget);
    ~GraphicsNodeItem();
    void observedCellDeleted(Cell *);

    AnimatedCycleNode * node() const;

    GraphicsNodeItem * next();
    GraphicsNodeItem * previous();
    GraphicsNodeItem * before();
    GraphicsNodeItem * after();

    bool isMoved() const;

    double height() const;
    double width() const;

    void setHeight(int i);
    void setWidth(double w);
    void setFixedY(double y);

    void updateText();

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent * event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);

private:
    void setPath_();

    AnimatedCycleNode * node_;
    QGraphicsTextItem * text_;
    AnimatedCycleWidget * widget_;

    bool isMoved_;
    double width_;
    double height_;
    double y_;
};

class GraphicsArrowItem: public QGraphicsPolygonItem
{
public:
    // Enable the use of qgraphicsitem_cast<>
    enum { Type = UserType + 2 };
    int type() const { return Type; }

    GraphicsArrowItem();
    void setEndPoints(const QPointF & p1, const QPointF & p2);
};

class AnimatedCycleWidget;
class AnimatedCycleGraphicsView: public QGraphicsView
{
public:
    AnimatedCycleGraphicsView(QGraphicsScene * scene, AnimatedCycleWidget * animatedCycleWidget);

    GraphicsNodeItem * nodeItemAt(const QPoint &pos);
    GraphicsArrowItem * arrowItemAt(const QPoint &pos);

protected:
    virtual void mousePressEvent(QMouseEvent * event);
    virtual void mouseMoveEvent(QMouseEvent * event);
    virtual void mouseReleaseEvent(QMouseEvent * event);
    virtual void wheelEvent(QWheelEvent *event);
    virtual void paintEvent(QPaintEvent * event);

private:
    AnimatedCycleWidget * animatedCycleWidget_;

    GraphicsNodeItem * itemAtPress_;
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
    void clearScene();

    void observedCellDeleted(Cell *);

    // Get current animated cycle
    // Notes: * The returned animated cycle is only aware of the nodes connected to first.
    //        * The returned animated cycle might be in an invalid state
    AnimatedCycle getAnimatedCycle() const;

    // Set current animated cycle

    GraphicsNodeItem * next(GraphicsNodeItem * item);
    GraphicsNodeItem * previous(GraphicsNodeItem * item);
    GraphicsNodeItem * before(GraphicsNodeItem * item);
    GraphicsNodeItem * after(GraphicsNodeItem * item);

    void start();
    void stop();

    void setReadOnly(bool b);
    bool isReadOnly() const;

    void setPrevious(GraphicsNodeItem * item, GraphicsNodeItem * itemPrevious);
    void setNext(GraphicsNodeItem * item, GraphicsNodeItem * itemNext);
    void setBefore(GraphicsNodeItem * item, GraphicsNodeItem * itemBefore);
    void setAfter(GraphicsNodeItem * item, GraphicsNodeItem * itemAfter);
    void deleteArrow(GraphicsArrowItem * arrowItem);
    void deleteItem(GraphicsNodeItem * item);

protected:
    virtual void mousePressEvent(QMouseEvent * event);

signals:

public slots:
    void load();
    void save();

private slots:
    void animate();

    void addSelectedCells();


private:
    void createNodeAndItem(Cell * cell);
    void createItem(AnimatedCycleNode * node);
    void computeItemHeightAndY();

    void computeSceneFromAnimatedCycle();

    QGraphicsScene * scene_;
    AnimatedCycleGraphicsView * view_;

    AnimatedCycle animatedCycle_;
    QMap<AnimatedCycleNode*, GraphicsNodeItem*> nodeToItem_;
    QMap<GraphicsNodeItem*, GraphicsArrowItem*> itemToNextArrow_;
    QMap<GraphicsNodeItem*, GraphicsArrowItem*> itemToPreviousArrow_;
    QMap<GraphicsNodeItem*, GraphicsArrowItem*> itemToNextArrowBorder_;
    QMap<GraphicsNodeItem*, GraphicsArrowItem*> itemToPreviousArrowBorder_;
    QMap<GraphicsNodeItem*, GraphicsArrowItem*> itemToAfterArrow_;
    QMap<GraphicsNodeItem*, GraphicsArrowItem*> itemToBeforeArrow_;

    QTimer timer_;

    bool isReadOnly_;

    // Connection with ST-VGC
    InbetweenFace * inbetweenFace_;
    int indexCycle_;

    QHBoxLayout * editorButtons_;

};

#endif // ANIMATEDCYCLEWIDGET_H
