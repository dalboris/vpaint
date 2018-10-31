// Copyright (C) 2012-2018 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "LayersWidget.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

namespace
{
const QColor layerColorIfCurrent = QColor::fromRgb(128, 190, 230);
const QColor layerColorIfNotCurrent = QColor::fromRgb(255, 255, 255);
const QColor layerListBackgroundColor = QColor::fromRgb(255, 255, 255);
} // namespace

LayerWidget::LayerWidget(int index, bool isCurrent) :
    index_(index),
    isCurrent_(isCurrent)
{
    checkBox_ = new QCheckBox();
    checkBox_->setCheckState(Qt::Checked);
    checkBox_->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    nameLabel_ = new QLabel();
    setName( tr("Layer %1").arg(index));
    QHBoxLayout * layout = new QHBoxLayout();
    layout->addWidget(checkBox_);
    layout->addWidget(nameLabel_);
    setLayout(layout);

    setAutoFillBackground(true);
    updateBackground_();
}

LayerWidget::~LayerWidget()
{

}

QString LayerWidget::name() const
{
    return nameLabel_->text();
}

void LayerWidget::setName(const QString& name)
{
    nameLabel_->setText(name);
}

int LayerWidget::index() const
{
    return index_;
}

void LayerWidget::setIndex(int index)
{
    index_ = index;
}

bool LayerWidget::isCurrent() const
{
    return isCurrent_;
}

void LayerWidget::setCurrent(bool b)
{
    isCurrent_ = b;
    updateBackground_();
}

void LayerWidget::mousePressEvent(QMouseEvent*)
{
    emit requestCurrent(index());
}

void LayerWidget::updateBackground_()
{
    QColor color = isCurrent() ? layerColorIfCurrent : layerColorIfNotCurrent;

    QPalette p = palette();
    p.setColor(QPalette::Background, color);
    setPalette(p);
}

LayersWidget::LayersWidget() :
    numVisibleLayerWidgets_(0),
    currentLayer_(nullptr)
{
    // VBoxLayout with all the individual LayerWidget instances
    layerListLayout_ = new QVBoxLayout();
    layerListLayout_->setContentsMargins(0,0,0,0);
    layerListLayout_->setSpacing(0);

    // Create a few LayerWidget instances for testing
    // XXX Why is this not working when I move this to the end of this function??
    createNewLayerWidget_();
    createNewLayerWidget_();
    createNewLayerWidget_();
    setCurrentLayer_(numVisibleLayerWidgets_ - 1);

    // Wrap the layerListLayout_ into yet another VBoxLayout.
    // We need this because:
    // 1. We need scrollArea->setWidgetResizable(true) to enable horizontal stretching
    //    of the LayerWidget items, so that the background color takes all the
    //    horizontal space when selected.
    // 2. Unfortunately, as a side effect, this enables vertical stretching too, which results
    //    in ugly vertical stretching of all the LayerWidget items.
    // 3. So we add a QSpacerItem to "eat" all the remaining space below layerListLayout_.
    QVBoxLayout * layerListLayout2 = new QVBoxLayout();
    layerListLayout2->setContentsMargins(0,0,0,0);
    layerListLayout2->setSpacing(0);
    layerListLayout2->addLayout(layerListLayout_);
    layerListLayout2->addStretch();

    // Put the vbox layout in a scrollarea
    QScrollArea * scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    QWidget * layerList = new QWidget();
    layerList->setLayout(layerListLayout2);
    scrollArea->setWidget(layerList);

    // Set background color for scrollarea
    QPalette p = scrollArea->palette();
    p.setColor(QPalette::Background, layerListBackgroundColor);
    scrollArea->setPalette(p);
    scrollArea->setAutoFillBackground(true);

    // Create buttons
    QPushButton * newLayerButton = new QPushButton(tr("New"));
    QPushButton * moveLayerUpButton = new QPushButton(tr("Move Up"));
    QPushButton * moveLayerDownButton = new QPushButton(tr("Move Down"));
    QPushButton * deleteLayerButton = new QPushButton(tr("Delete"));
    connect(newLayerButton, &QPushButton::clicked, this, &LayersWidget::onNewLayerClicked_);
    connect(moveLayerUpButton, &QPushButton::clicked, this, &LayersWidget::onMoveLayerUpClicked_);
    connect(moveLayerDownButton, &QPushButton::clicked, this, &LayersWidget::onMoveLayerDownClicked_);
    connect(deleteLayerButton, &QPushButton::clicked, this, &LayersWidget::onDeleteLayerClicked_);
    QHBoxLayout * buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(newLayerButton);
    buttonsLayout->addWidget(moveLayerUpButton);
    buttonsLayout->addWidget(moveLayerDownButton);
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(deleteLayerButton);

    // Add scrollarea to this widget
    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget(scrollArea);
    layout->addLayout(buttonsLayout);
    setLayout(layout);
}

LayersWidget::~LayersWidget()
{

}

void LayersWidget::setCurrentLayer_(int index)
{
    // Early return if already current
    if (currentLayer_ && currentLayer_->index() == index) {
        return;
    }

    // Set current layer (if any) as not current anymore
    if (currentLayer_) {
        currentLayer_->setCurrent(false);
        currentLayer_ = nullptr;
    }

    // Set new current layer if provided index is valid
    size_t index_ = index;
    if (0 <= index && index < numVisibleLayerWidgets_) {
        LayerWidget * newCurrentLayer = layerWidgets_[index_];
        newCurrentLayer->setCurrent(true);
        currentLayer_ = newCurrentLayer;
    }
}

void LayersWidget::onNewLayerClicked_()
{
    // Show or create one more LayerWidget
    if (numVisibleLayerWidgets_ < static_cast<int>(layerWidgets_.size())) {
        layerWidgets_[numVisibleLayerWidgets_]->show();
        ++numVisibleLayerWidgets_;
    }
    else {
        createNewLayerWidget_();
    }

    // Insert above current layer, or keep last if no current layer
    int newCurrentIndex = numVisibleLayerWidgets_ - 1;
    if (currentLayer_) {
        newCurrentIndex = currentLayer_->index();
        for (int i = numVisibleLayerWidgets_ - 1; i > newCurrentIndex; --i) {
            layerWidgets_[i]->setName(layerWidgets_[i-1]->name());
        }
    }
    layerWidgets_[newCurrentIndex]->setName("Layer");
    setCurrentLayer_(newCurrentIndex);
}

void LayersWidget::onDeleteLayerClicked_()
{
    if (currentLayer_) {
        int currentIndex = currentLayer_->index();
        for (int i = currentIndex; i < numVisibleLayerWidgets_ - 1; ++i) {
            layerWidgets_[i]->setName(layerWidgets_[i+1]->name());
        }
        layerWidgets_[numVisibleLayerWidgets_ - 1]->hide();
        --numVisibleLayerWidgets_;

        // Set layer below as current, unless it was the bottom layer,
        // in which case we set the layer above as current
        if (currentIndex < numVisibleLayerWidgets_) {
            setCurrentLayer_(currentIndex);
        }
        else {
            setCurrentLayer_(currentIndex - 1);
        }
    }
}

void LayersWidget::onMoveLayerUpClicked_()
{
    if (currentLayer_) {
        int i = currentLayer_->index();
        if (i > 0) {
            int j = i - 1;
            QString temp = layerWidgets_[j]->name();
            layerWidgets_[j]->setName(layerWidgets_[i]->name());
            layerWidgets_[i]->setName(temp);
            setCurrentLayer_(j);
        }
    }
}

void LayersWidget::onMoveLayerDownClicked_()
{
    if (currentLayer_) {
        int i = currentLayer_->index();
        if (i < numVisibleLayerWidgets_ - 1) {
            int j = i + 1;
            QString temp = layerWidgets_[j]->name();
            layerWidgets_[j]->setName(layerWidgets_[i]->name());
            layerWidgets_[i]->setName(temp);
            setCurrentLayer_(j);
        }
    }
}

void LayersWidget::createNewLayerWidget_()
{
    // Create layer
    LayerWidget * layer = new LayerWidget(layerWidgets_.size());
    ++numVisibleLayerWidgets_;
    layerWidgets_.push_back(layer);
    layerListLayout_->addWidget(layer);
    connect(layer, &LayerWidget::requestCurrent, this, &LayersWidget::setCurrentLayer_);
}

void LayersWidget::destroyLastLayerWidget_()
{
    // Get layer to destroy
    LayerWidget * layer = layerWidgets_.back();

    // If it was the current layer, set the layer above (if any) as current
    if (layer->isCurrent()) {
        setCurrentLayer_(layer->index() - 1); // safe even when index == 0
    }

    // If it was visible, decrease the number of visible LayerWidgets
    if (layer->isVisible()) {
        --numVisibleLayerWidgets_;
    }

    // Destroy layer
    disconnect(layer, &LayerWidget::requestCurrent, this, &LayersWidget::setCurrentLayer_);
    layerListLayout_->takeAt(layer->index());
    layerWidgets_.pop_back();
    delete layer;
}
