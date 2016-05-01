// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OBJECTPROPERTIESWIDGET_H
#define OBJECTPROPERTIESWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>

#include "VectorAnimationComplex/CellList.h"
#include "AnimatedCycleWidget.h"

class ObjectPropertiesWidget: public QWidget
{
    Q_OBJECT

public:
    ObjectPropertiesWidget();
    ~ObjectPropertiesWidget();

    void setObjects(const VectorAnimationComplex::CellSet & cells);

private slots:
    void idMoreLessSlot();
    void toggleAnimatedCycleShowHide();
    void setAnimatedCycle(int i);
    void animatedCycleEdit();
    void animatedCycleNew();
    void animatedCycleDelete();

    void updateBeforeCycleStartingPoint(int i);
    void updateAfterCycleStartingPoint(int i);
    void sliderStartingPointReleased();

private:

    QString getStringType(VectorAnimationComplex::Cell * cell);
    QString getStringType(const VectorAnimationComplex::CellSet & cells);
    void setType(const QString & type);
    void setId(const VectorAnimationComplex::CellSet & cells);
    void setIdFromString();

    QVBoxLayout * mainLayout_;
    QLabel * type_;
    QLabel * id_;
    QString idLess_;
    QString moreText_;
    QString lessText_;
    QString idMore_;
    QPushButton * idMoreLessButton_;

    // Inbetween edge
    void setObject(InbetweenEdge * inbetweenEdge);
    void hideInbetweenClosedEdgeWidgets();
    void showInbetweenClosedEdgeWidgets();
    InbetweenEdge * inbetweenEdge_;
    QSlider * inbetweenClosedEdgeBeforeCycleSlider_;
    QSlider * inbetweenClosedEdgeAfterCycleSlider_;
    QWidget * inbetweenClosedEdgeWidgets_;

    // Inbetween face
    void setObject(InbetweenFace * inbetweenFace);
    void hideAnimatedCycleWidgets();
    void showAnimatedCycleWidgets();
    void hideAnimatedCycleWidget();
    void showAnimatedCycleWidget();
    void updateAnimatedCycleComboBox();
    InbetweenFace * inbetweenFace_;
    QWidget * animatedCycleWidgets_;
    QPushButton * animatedCycleShowHide_;
    QPushButton * animatedCycleEdit_;
    QPushButton * animatedCycleNew_;
    QPushButton * animatedCycleDelete_;
    QComboBox * animatedCycleComboBox_;

    AnimatedCycleWidget * animatedCycleWidget_;
    QWidget * bottomStretch_;

};

#endif // OBJECTPROPERTIESWIDGET_H
