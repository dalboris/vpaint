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

#include "ObjectPropertiesWidget.h"
#include "VectorAnimationComplex/VAC.h"
#include "VectorAnimationComplex/Cell.h"
#include "VectorAnimationComplex/KeyVertex.h"
#include "VectorAnimationComplex/KeyEdge.h"
#include "VectorAnimationComplex/KeyFace.h"
#include "VectorAnimationComplex/InbetweenVertex.h"
#include "VectorAnimationComplex/InbetweenEdge.h"
#include "VectorAnimationComplex/InbetweenFace.h"
#include "Global.h"
#include "MainWindow.h"

ObjectPropertiesWidget::ObjectPropertiesWidget()
{
    // ---- Type ----
    QLabel * typeLabel = new QLabel(tr("Type:"));
    type_ = new QLabel(tr("no objects"));
    QHBoxLayout * typeLayout = new QHBoxLayout();
    typeLayout->addWidget(typeLabel);
    typeLayout->setAlignment(typeLabel, Qt::AlignTop);
    typeLayout->addWidget(type_);
    typeLayout->addStretch();

    // ---- ID ----
    moreText_ = tr("more");
    lessText_ = tr("less");
    QLabel * idLabel = new QLabel(tr("ID:"));
    id_ = new QLabel("");
    idMoreLessButton_ = new QPushButton(moreText_);
    idMoreLessButton_->setMaximumHeight(15);
    connect(idMoreLessButton_, SIGNAL(clicked()), this, SLOT(idMoreLessSlot()));
    QHBoxLayout * idLayout = new QHBoxLayout();
    idLayout->addWidget(idLabel);
    idLayout->setAlignment(idLabel, Qt::AlignTop);
    idLayout->addWidget(id_);
    idLayout->setAlignment(id_, Qt::AlignTop);
    idLayout->addWidget(idMoreLessButton_);
    idLayout->setAlignment(idMoreLessButton_, Qt::AlignTop);
    idLayout->addStretch();

    // -- inbetween closed edge --
    inbetweenClosedEdgeWidgets_ = new QWidget();
    inbetweenClosedEdgeBeforeCycleSlider_ = new QSlider(Qt::Horizontal);
    inbetweenClosedEdgeBeforeCycleSlider_->setRange(0,100);
    connect(inbetweenClosedEdgeBeforeCycleSlider_, SIGNAL(sliderMoved(int)), this, SLOT(updateBeforeCycleStartingPoint(int)));
    connect(inbetweenClosedEdgeBeforeCycleSlider_, SIGNAL(sliderReleased()), this, SLOT(sliderStartingPointReleased()));
    inbetweenClosedEdgeAfterCycleSlider_ = new QSlider(Qt::Horizontal);
    inbetweenClosedEdgeAfterCycleSlider_->setRange(0,100);
    connect(inbetweenClosedEdgeAfterCycleSlider_, SIGNAL(sliderMoved(int)), this, SLOT(updateAfterCycleStartingPoint(int)));
    connect(inbetweenClosedEdgeAfterCycleSlider_, SIGNAL(sliderReleased()), this, SLOT(sliderStartingPointReleased()));
    QGridLayout * inbetweenClosedEdgeLayout = new QGridLayout();
    //inbetweenClosedEdgeLayout->addWidget(new QLabel("before cycle starting point:"),0,0);
    inbetweenClosedEdgeLayout->addWidget(new QLabel("Cycle offset:"),1,0);
    //inbetweenClosedEdgeLayout->addWidget(inbetweenClosedEdgeBeforeCycleSlider_,0,1);
    inbetweenClosedEdgeLayout->addWidget(inbetweenClosedEdgeAfterCycleSlider_,1,1);
    inbetweenClosedEdgeWidgets_->setLayout(inbetweenClosedEdgeLayout);

    // -- animated cycle --
    animatedCycleComboBox_ = new QComboBox();
    animatedCycleShowHide_ = new QPushButton("Show");
    connect(animatedCycleShowHide_, SIGNAL(clicked()), this, SLOT(toggleAnimatedCycleShowHide()));
    animatedCycleEdit_ = new QPushButton("Edit");
    connect(animatedCycleEdit_, SIGNAL(clicked()), this, SLOT(animatedCycleEdit()));
    animatedCycleNew_ = new QPushButton("New");
    connect(animatedCycleNew_, SIGNAL(clicked()), this, SLOT(animatedCycleNew()));
    animatedCycleDelete_ = new QPushButton("Delete");
    connect(animatedCycleDelete_, SIGNAL(clicked()), this, SLOT(animatedCycleDelete()));
    animatedCycleWidget_ = new AnimatedCycleWidget();
    animatedCycleWidget_->hide();

    animatedCycleWidgets_ = new QWidget();
    QWidget * hStretch = new QWidget();
    hStretch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    QHBoxLayout * animatedCycleButtonsLayout = new QHBoxLayout();
    animatedCycleButtonsLayout->addWidget(animatedCycleComboBox_);
    animatedCycleComboBox_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    connect(animatedCycleComboBox_, SIGNAL(currentIndexChanged(int)), this, SLOT(setAnimatedCycle(int)));
    animatedCycleButtonsLayout->addWidget(animatedCycleShowHide_);
    animatedCycleButtonsLayout->addWidget(animatedCycleEdit_);
    animatedCycleButtonsLayout->addWidget(animatedCycleNew_);
    animatedCycleButtonsLayout->addWidget(animatedCycleDelete_);
    animatedCycleButtonsLayout->addStretch();
    QVBoxLayout * animatedCycleWidgetsLayout = new QVBoxLayout();
    animatedCycleWidgetsLayout->addLayout(animatedCycleButtonsLayout);
    animatedCycleWidgetsLayout->addWidget(animatedCycleWidget_);
    animatedCycleWidgets_->setLayout(animatedCycleWidgetsLayout);

    // -- Main layout --
    mainLayout_ = new QVBoxLayout();
    mainLayout_->addLayout(typeLayout);
    mainLayout_->addLayout(idLayout);
    mainLayout_->addWidget(inbetweenClosedEdgeWidgets_);
    mainLayout_->addWidget(animatedCycleWidgets_);
    bottomStretch_ = new QWidget();
    bottomStretch_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout_->addWidget(bottomStretch_);
    //mainLayout_->addStretch();
    setLayout(mainLayout_);

    // -- Set that no object is selected --
    setObjects(VectorAnimationComplex::CellSet());
}

ObjectPropertiesWidget::~ObjectPropertiesWidget()
{

}

void ObjectPropertiesWidget::hideAnimatedCycleWidget()
{
    animatedCycleWidget_->hide();
    animatedCycleWidget_->stop();
    animatedCycleShowHide_->setText(tr("Show"));
    bottomStretch_->show();
}

void ObjectPropertiesWidget::showAnimatedCycleWidget()
{
    animatedCycleWidget_->show();
    animatedCycleWidget_->start();
    animatedCycleShowHide_->setText(tr("Hide"));
    bottomStretch_->hide();
}

void ObjectPropertiesWidget::toggleAnimatedCycleShowHide()
{
    if(animatedCycleWidget_->isVisible())
    {
        hideAnimatedCycleWidget();
    }
    else
    {
        showAnimatedCycleWidget();
    }
}

void ObjectPropertiesWidget::animatedCycleEdit()
{
    global()->mainWindow()->editAnimatedCycle(inbetweenFace_,animatedCycleComboBox_->currentIndex());
    hideAnimatedCycleWidget();
}

void ObjectPropertiesWidget::animatedCycleNew()
{
    inbetweenFace_->addAnimatedCycle();
    updateAnimatedCycleComboBox();

    animatedCycleComboBox_->setCurrentIndex(inbetweenFace_->numAnimatedCycles()-1);
    animatedCycleEdit();

    VectorAnimationComplex::VAC * vac = inbetweenFace_->vac();
    emit vac->needUpdatePicking();
    emit vac->changed();
    emit vac->checkpoint();
}

void ObjectPropertiesWidget::animatedCycleDelete()
{
    int i = animatedCycleComboBox_->currentIndex();
    int n = inbetweenFace_->numAnimatedCycles();
    if(0<=i && i<n)
        inbetweenFace_->removeCycle(i);
    updateAnimatedCycleComboBox();

    VectorAnimationComplex::VAC * vac = inbetweenFace_->vac();
    emit vac->needUpdatePicking();
    emit vac->changed();
    emit vac->checkpoint();
}

void ObjectPropertiesWidget::updateBeforeCycleStartingPoint(int i)
{
    if(inbetweenEdge_ && inbetweenEdge_->isClosed())
    {
        inbetweenEdge_->setBeforeCycleStartingPoint( (double)i / 100.0);

        VectorAnimationComplex::VAC * vac = inbetweenEdge_->vac();
        emit vac->needUpdatePicking();
        emit vac->changed();
    }
}

void ObjectPropertiesWidget::updateAfterCycleStartingPoint(int i)
{
    if(inbetweenEdge_ && inbetweenEdge_->isClosed())
    {
        inbetweenEdge_->setAfterCycleStartingPoint( (double)i / 100.0);

        VectorAnimationComplex::VAC * vac = inbetweenEdge_->vac();
        emit vac->needUpdatePicking();
        emit vac->changed();
    }
}

void ObjectPropertiesWidget::sliderStartingPointReleased()
{

    if(inbetweenEdge_ && inbetweenEdge_->isClosed())
    {
        VectorAnimationComplex::VAC * vac = inbetweenEdge_->vac();
        emit vac->checkpoint();
    }
}

void ObjectPropertiesWidget::hideInbetweenClosedEdgeWidgets()
{
    inbetweenClosedEdgeWidgets_->hide();
    bottomStretch_->show();
}

void ObjectPropertiesWidget::showInbetweenClosedEdgeWidgets()
{
    inbetweenClosedEdgeWidgets_->show();
    bottomStretch_->show();
}

void ObjectPropertiesWidget::hideAnimatedCycleWidgets()
{
    animatedCycleWidgets_->hide();
    animatedCycleWidget_->stop();
    animatedCycleWidget_->setAnimatedCycle(AnimatedCycle());
    bottomStretch_->show();
}

void ObjectPropertiesWidget::showAnimatedCycleWidgets()
{
    animatedCycleWidgets_->show();
    if(animatedCycleWidget_->isVisible())
    {
        animatedCycleWidget_->start();
        bottomStretch_->hide();
    }
}

void ObjectPropertiesWidget::setAnimatedCycle(int i)
{
    if(i>=0 && i<inbetweenFace_->numAnimatedCycles())
        animatedCycleWidget_->setAnimatedCycle(inbetweenFace_, i);
}

void ObjectPropertiesWidget::updateAnimatedCycleComboBox()
{
    animatedCycleComboBox_->clear();
    for(int i=1; i<=inbetweenFace_->numAnimatedCycles(); ++i)
    {
        animatedCycleComboBox_->addItem(tr("Animated Cycle %1").arg(i));
    }
}

void ObjectPropertiesWidget::setObject(InbetweenEdge * inbetweenEdge)
{
    inbetweenEdge_ = inbetweenEdge;

    if(inbetweenEdge_->isClosed())
    {
        inbetweenClosedEdgeBeforeCycleSlider_->setValue((int) (inbetweenEdge_->beforeCycleStartingPoint() * 100) );
        inbetweenClosedEdgeAfterCycleSlider_->setValue((int) (inbetweenEdge_->afterCycleStartingPoint() * 100) );
        showInbetweenClosedEdgeWidgets();
    }
    else
    {

    }
}

void ObjectPropertiesWidget::setObject(InbetweenFace * inbetweenFace)
{
    inbetweenFace_ = inbetweenFace;

    showAnimatedCycleWidgets();
    updateAnimatedCycleComboBox();

    animatedCycleComboBox_->setCurrentIndex(0);
    setAnimatedCycle(0);
}

void ObjectPropertiesWidget::setObjects(const VectorAnimationComplex::CellSet & cells)
{
    // Always visible info
    setType(getStringType(cells));
    setId(cells);

    // Hide all other selection-dependent info
    hideInbetweenClosedEdgeWidgets();
    hideAnimatedCycleWidgets();

    // set pointers to null
    inbetweenEdge_ = 0;
    inbetweenFace_ = 0;

    // Show selection-dependent info
    if(cells.size() == 1)
    {
        Cell * cell = *cells.begin();
        InbetweenEdge * sedge = cell->toInbetweenEdge();
        InbetweenFace * sface = cell->toInbetweenFace();
        if(sedge)
        {
            setObject(sedge);
        }
        else if(sface)
        {
            setObject(sface);
        }
    }
}


QString ObjectPropertiesWidget::getStringType(VectorAnimationComplex::Cell * cell)
{
    if(cell->toKeyVertex())
        return tr("key vertex");
    else if(cell->toKeyEdge())
        return tr("key edge");
    else if(cell->toKeyFace())
        return tr("key face");
    else if(cell->toInbetweenVertex())
        return tr("inbetween vertex");
    else if(cell->toInbetweenEdge())
        return tr("inbetween edge");
    else if(cell->toInbetweenFace())
        return tr("inbetween face");
    else
        return tr("unknown object");
}

QString ObjectPropertiesWidget::getStringType(const VectorAnimationComplex::CellSet & cells)
{
    //VectorAnimationComplex::KeyVertexSet kv = cells;
    //VectorAnimationComplex::KeyEdgeSet ke = cells;
    //VectorAnimationComplex::KeyFaceSet kf = cells;
    //VectorAnimationComplex::InbetweenVertexSet sv = cells;
    //VectorAnimationComplex::InbetweenEdgeSet se = cells;
    //VectorAnimationComplex::InbetweenFaceSet sf = cells;

    // Count cells
    int nkv = 0;
    int nkoe = 0;
    int nkce = 0;
    int nkf = 0;
    int nsv = 0;
    int nsoe = 0;
    int nsce = 0;
    int nsf = 0;
    for(VectorAnimationComplex::Cell * cell: cells)
    {
        if(cell->toKeyVertex())
            ++nkv;
        else if(cell->toKeyEdge())
        {
            if(cell->toKeyEdge()->isClosed())
                ++nkce;
            else
                ++nkoe;
        }
        else if(cell->toKeyFace())
            ++nkf;
        else if(cell->toInbetweenVertex())
            ++nsv;
        else if(cell->toInbetweenEdge())
        {
            if(cell->toInbetweenEdge()->isClosed())
                ++nsce;
            else
                ++nsoe;
        }
        else if(cell->toInbetweenFace())
            ++nsf;
    }

    // Set string according to count
    QStringList stringList;
    if(nkv == 1)
        stringList << tr("1 key vertex");
    if(nkv > 1)
        stringList << QString().setNum(nkv) + tr(" key vertices");
    if(nkce == 1)
        stringList << tr("1 key closed edge");
    if(nkce > 1)
        stringList << QString().setNum(nkce) + tr(" key closed edges");
    if(nkoe == 1)
        stringList << tr("1 key open edge");
    if(nkoe > 1)
        stringList << QString().setNum(nkoe) + tr(" key open edges");
    if(nkf == 1)
        stringList << tr("1 key face");
    if(nkf > 1)
        stringList << QString().setNum(nkf) + tr(" key faces");
    if(nsv == 1)
        stringList << tr("1 inbetween vertex");
    if(nsv > 1)
        stringList << QString().setNum(nsv) + tr(" inbetween vertices");
    if(nsce == 1)
        stringList << tr("1 inbetween closed edge");
    if(nsce > 1)
        stringList << QString().setNum(nsce) + tr(" inbetween closed edges");
    if(nsoe == 1)
        stringList << tr("1 inbetween open edge");
    if(nsoe > 1)
        stringList << QString().setNum(nsoe) + tr(" inbetween open edges");
    if(nsf == 1)
        stringList << tr("1 inbetween face");
    if(nsf > 1)
        stringList << QString().setNum(nsf) + tr(" inbetween faces");

    if(stringList.isEmpty())
        return tr("no objects");
    else
    {
        QString res = stringList[0];
        for(int i=1; i<stringList.size(); ++i)
            res += QString("\n") + stringList[i];
        return res;
    }
}

void ObjectPropertiesWidget::setType(const QString & type)
{
    type_->setText(type);
}

void ObjectPropertiesWidget::setId(const VectorAnimationComplex::CellSet & cells)
{
    int numIdPerLine = 5;

    idLess_ = QString("");
    idMore_ = QString("");
    idMoreLessButton_->hide();

    int i = 0; // count cells
    int j = 0; // num on line
    for(VectorAnimationComplex::Cell * cell: cells)
    {
        j = i % numIdPerLine;
        if(i==0)
        {
            idMore_ += QString().setNum(cell->id());
            idLess_ += QString().setNum(cell->id());
        }
        else if(j==0)
        {
            idMore_ += QString(",\n") + QString().setNum(cell->id());

            if(i == numIdPerLine)
            {
                idLess_ += QString(",...");
                idMoreLessButton_->show();
            }
        }
        else
        {
            idMore_ += QString(", ") + QString().setNum(cell->id());

            if(i < numIdPerLine)
            {
                idLess_ += QString(", ") + QString().setNum(cell->id());
            }


        }
        ++i;
    }

    setIdFromString();
}

void ObjectPropertiesWidget::setIdFromString()
{
    if(idMoreLessButton_->text() == moreText_)
    {
        id_->setText(idLess_);
    }
    else
    {
        id_->setText(idMore_);
    }
    update();
}

void ObjectPropertiesWidget::idMoreLessSlot()
{
    if(idMoreLessButton_->text() ==  moreText_)
    {
        idMoreLessButton_->setText(lessText_);
    }
    else
    {
        idMoreLessButton_->setText(moreText_);
    }
    setIdFromString();
}
