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
#include <QScrollArea>
#include <QVBoxLayout>

namespace
{
const QColor layerColorIfCurrent = QColor::fromRgb(128, 190, 230);
const QColor layerColorIfNotCurrent = QColor::fromRgb(255, 255, 255);
const QColor layerListBackgroundColor = QColor::fromRgb(255, 255, 255);
} // namespace

LayerWidget::LayerWidget(int layerIndex, bool isCurrent) :
    index_(layerIndex),
    isCurrent_(isCurrent)
{
    checkBox_ = new QCheckBox();
    checkBox_->setCheckState(Qt::Checked);
    checkBox_->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    label_ = new QLabel();
    QHBoxLayout * layout = new QHBoxLayout();
    layout->addWidget(checkBox_);
    layout->addWidget(label_);
    setLayout(layout);

    setAutoFillBackground(true);

    updateBackground_();
    updateCheckBoxState_();
    updateLabelText_();
}

LayerWidget::~LayerWidget()
{

}

int LayerWidget::index() const
{
    return index_;
}

void LayerWidget::setIndex(int index)
{
    index_ = index;
    updateLabelText_();
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

void LayerWidget::updateCheckBoxState_()
{
    // TODO
}

void LayerWidget::updateLabelText_()
{
    label_->setText(tr("Layer %1").arg(index()));
}

LayersWidget::LayersWidget()
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

    // Add scrollarea to this widget
    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget(scrollArea);
    setLayout(layout);
}

LayersWidget::~LayersWidget()
{

}

void LayersWidget::createNewLayerWidget_()
{
    LayerWidget * layer = new LayerWidget(layers_.size());
    layers_.push_back(layer);
    layerListLayout_->addWidget(layer);
    connect(layer, &LayerWidget::requestCurrent, this, &LayersWidget::onRequestCurrent_);

    onRequestCurrent_(layer->index());
}

void LayersWidget::destroyLastLayerWidget_()
{
    // Get layer to destroy
    LayerWidget * layer = layers_.back();

    // Set new current layer
    if (layer->isCurrent() && layer->index() > 0) {
        layers_[0]->setCurrent(true);
    }

    // Destroy layer
    disconnect(layer, &LayerWidget::requestCurrent, this, &LayersWidget::onRequestCurrent_);
    layerListLayout_->takeAt(layer->index());
    layers_.pop_back();
    delete layer;
}

void LayersWidget::onRequestCurrent_(int index)
{
    size_t index_ = index;
    layers_[index_]->setCurrent(true);
    for (size_t i = 0; i < layers_.size(); ++i) {
        if (i != index_ && layers_[i]->isCurrent()) {
            layers_[i]->setCurrent(false);
        }
    }
}
