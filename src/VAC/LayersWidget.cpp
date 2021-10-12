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

#include "LayersWidget.h"

#include <cmath>

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

#include "Layer.h"
#include "Scene.h"

namespace impl_
{

LayerWidget::LayerWidget(int index) :
    index_(index),
    isActive_(false)
{
    visibilityCheckBox_ = new QCheckBox();
    visibilityCheckBox_->setCheckState(Qt::Checked);
    visibilityCheckBox_->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    connect(visibilityCheckBox_, SIGNAL(stateChanged(int)), this, SLOT(onVisibilityCheckBoxStateChanged_(int)));
    connect(visibilityCheckBox_, SIGNAL(clicked(bool)), this, SLOT(onVisibilityCheckBoxClicked_(bool)));

    nameLabel_ = new QLabel();
    nameLabel_->setMinimumHeight(30);

    nameLineEdit_ = new QLineEdit();
    nameLineEdit_->setMinimumHeight(30);
    nameLineEdit_->hide();
    connect(nameLineEdit_, &QLineEdit::editingFinished, this, &LayerWidget::onNameLineEditEditingFinished_);

    QHBoxLayout * layout = new QHBoxLayout();
    layout->addWidget(visibilityCheckBox_);
    layout->addWidget(nameLabel_);
    layout->addWidget(nameLineEdit_);
    setLayout(layout);

    setAutoFillBackground(true);
    updateBackground_();
}

LayerWidget::~LayerWidget()
{

}

int LayerWidget::index() const
{
    return index_;
}

bool LayerWidget::isActive() const
{
    return isActive_;
}

void LayerWidget::setActive(bool b)
{
    if (b != isActive_)
    {
        isActive_ = b;
        updateBackground_();
        if (isActive_)
        {
            emit activated(index());
        }
    }
}

bool LayerWidget::visibility() const
{
    return visibilityCheckBox_->isChecked();
}

void LayerWidget::setVisibility(bool b)
{
    if (b != visibility())
    {
        visibilityCheckBox_->setChecked(b);
        // Note: we don't emit a signal here, as it will be emitted
        // in onVisibilityCheckBoxStateChanged_.
    }
}

QString LayerWidget::name() const
{
    return nameLabel_->text();
}

bool LayerWidget::setName(const QString& newName)
{
    // Abord name editing if any
    abortNameEditing_();

    // Set new name if different form current name
    if (newName != name())
    {
        nameLabel_->setText(newName);
        emit nameChanged(index());
        return true;
    }
    else
    {
        return false;
    }
}

void LayerWidget::startNameEditing()
{
    startNameEditing_(NameEditingReason_::ExternalRequest);
}

void LayerWidget::mousePressEvent(QMouseEvent*)
{
    if (!isActive_)
    {
        setActive(true);
        emit checkpoint();
    }
}

void LayerWidget::mouseDoubleClickEvent(QMouseEvent*)
{
    startNameEditing_(NameEditingReason_::DoubleClick);
}

void LayerWidget::onVisibilityCheckBoxClicked_(bool)
{
    emit checkpoint();
}

void LayerWidget::onVisibilityCheckBoxStateChanged_(int)
{
    emit visibilityChanged(index());
}

void LayerWidget::onNameLineEditEditingFinished_()
{
    finishNameEditing_();
}

void LayerWidget::startNameEditing_(NameEditingReason_ reason)
{
    if (!nameLineEdit_->isVisible())
    {
        nameEditingReason_ = reason;
        nameLineEdit_->setText(name());
        nameLabel_->hide();
        nameLineEdit_->show();
        nameLineEdit_->selectAll();
        nameLineEdit_->setFocus();
    }
}

void LayerWidget::abortNameEditing_()
{
    if (nameLineEdit_->isVisible())
    {
        nameLineEdit_->hide();
        nameLabel_->show();
    }
}

void LayerWidget::finishNameEditing_()
{
    if (nameLineEdit_->isVisible())
    {
        QString newName = nameLineEdit_->text();

        nameLineEdit_->hide();
        nameLabel_->show();

        bool changed = setName(newName);
        if (changed && nameEditingReason_ == NameEditingReason_::DoubleClick)
        {
            // We only emit checkpoint if the user action causing the scene to
            // change is initiated from this LayerWidget. In other words, the
            // widget responsible for starting a user action is the widget
            // responsible for calling checkpoint.
            emit checkpoint();
        }

        if (nameEditingReason_ == NameEditingReason_::ExternalRequest)
        {
            emit nameEditingFinished(index());
        }
    }
}

void LayerWidget::updateBackground_()
{
    QPalette::ColorRole backgroundRole = isActive() ? QPalette::Highlight : QPalette::Base;
    setBackgroundRole(backgroundRole);
}

} // namespace impl_

LayersWidget::LayersWidget(Scene * scene) :
    scene_(scene),
    numVisibleLayerWidgets_(0),
    activeLayerWidget_(nullptr)
{
    // VBoxLayout with all the individual LayerWidget instances
    layerListLayout_ = new QVBoxLayout();
    layerListLayout_->setContentsMargins(0,0,0,0);
    layerListLayout_->setSpacing(0);

    // Create one LayerWidget right now. It will be hidden shortly after if the
    // scene has in fact no layers.
    //
    // This is required because for some reason, LayerWidgets won't show up if
    // none exist before layerListLayout_ is added to the scrollArea. I suspect
    // this to be a bug of Qt.
    //
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
    scrollArea->setBackgroundRole(QPalette::Base);
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

    // Connect to scene
    updateUiFromScene_();
    connect(scene_, SIGNAL(layerAttributesChanged()), this, SLOT(onSceneLayerAttributesChanged_()));
}

LayersWidget::~LayersWidget()
{

}

Scene * LayersWidget::scene() const
{
    return scene_;
}

void LayersWidget::onLayerWidgetActivated_(int index)
{
    scene()->setActiveLayer(numVisibleLayerWidgets_ - 1 - index);
}

void LayersWidget::onLayerWidgetVisibilityChanged_(int index)
{
    if (0 <= index && index < numVisibleLayerWidgets_) {
        int j = numVisibleLayerWidgets_ - 1 - index;
        bool visibility = layerWidgets_[index]->visibility();
        scene()->layer(j)->setVisible(visibility);
    }
}

void LayersWidget::onLayerWidgetNameChanged_(int index)
{
    if (0 <= index && index < numVisibleLayerWidgets_) {
        int j = numVisibleLayerWidgets_ - 1 - index;
        QString name = layerWidgets_[index]->name();
        scene()->layer(j)->setName(name);
    }
}

void LayersWidget::onLayerWidgetNameEditingFinished_(int)
{
    scene()->emitCheckpoint();
}

void LayersWidget::onLayerWidgetCheckpoint_()
{
    scene()->emitCheckpoint();
}

void LayersWidget::onNewLayerClicked_()
{
    // Create layer. This should indirectly create the corresponding
    // LayerWidget, unless using asynchronous signals/slots.
    Layer * layer = scene()->createLayer(tr("New Layer"));

    // Enter name editing mode. We need to check in case of asynchronous
    // signals/slots.
    //
    if (activeLayerWidget_)
    {
        int j = numVisibleLayerWidgets_ - 1 - activeLayerWidget_->index();
        if (scene()->layer(j) == layer)
        {
            activeLayerWidget_->startNameEditing();
            // Checkpoint will be emitted in onLayerWidgetNameEditingFinished_
        }
    }
    else
    {
        // This is not supposed to happen
        scene()->emitCheckpoint();
    }
}

void LayersWidget::onDeleteLayerClicked_()
{
    scene()->destroyActiveLayer();
    scene()->emitCheckpoint();
}

void LayersWidget::onMoveLayerUpClicked_()
{
    scene()->moveActiveLayerUp();
    scene()->emitCheckpoint();
}

void LayersWidget::onMoveLayerDownClicked_()
{
    scene()->moveActiveLayerDown();
    scene()->emitCheckpoint();
}

void LayersWidget::onSceneLayerAttributesChanged_()
{
    updateUiFromScene_();
}

void LayersWidget::updateUiFromScene_()
{
    // Show as many existing LayerWidgets as necessary
    const auto numLayers = scene()->numLayers();
    const auto numLayerWidgets = int(layerWidgets_.size());
    const auto newNumVisibleLayerWidgets = std::min(numLayers, numLayerWidgets);
    for (int i = numVisibleLayerWidgets_; i < newNumVisibleLayerWidgets; ++i) {
        layerWidgets_[i]->show();
        ++numVisibleLayerWidgets_;
    }

    // Create as many new LayerWidgets as necessary
    for (int i = numVisibleLayerWidgets_; i < numLayers; ++i) {
        createNewLayerWidget_();
    }

    // Hide superfluous LayerWidgets
    for (int i = numLayers; i < numVisibleLayerWidgets_; ++i) {
        layerWidgets_[i]->hide();
    }
    numVisibleLayerWidgets_ = numLayers;

    // Set LayerWidgets names and visibility
    for (int i = 0; i < numVisibleLayerWidgets_; ++i) {
        int j = numVisibleLayerWidgets_ - 1 - i;
        bool visibility= scene()->layer(j)->isVisible();
        QString name = scene()->layer(j)->name();
        layerWidgets_[i]->setVisibility(visibility);
        layerWidgets_[i]->setName(name);
    }

    // Set active LayerWidget
    int jActive = scene()->activeLayerIndex();
    int iActive = numVisibleLayerWidgets_ - 1 - jActive;
    if (activeLayerWidget_ && activeLayerWidget_->index() != iActive) {
        activeLayerWidget_->setActive(false);
        activeLayerWidget_ = nullptr;
    }
    if (0 <= iActive && iActive < numVisibleLayerWidgets_) {
        activeLayerWidget_ = layerWidgets_[iActive];
        activeLayerWidget_->setActive(true);
    }
}

// Precondition: all LayerWidgets are visible
void LayersWidget::createNewLayerWidget_()
{
    impl_::LayerWidget * layerWidget = new impl_::LayerWidget(int(layerWidgets_.size()));
    ++numVisibleLayerWidgets_;
    layerWidgets_.push_back(layerWidget);
    layerListLayout_->addWidget(layerWidget);
    connect(layerWidget, &impl_::LayerWidget::activated, this, &LayersWidget::onLayerWidgetActivated_);
    connect(layerWidget, &impl_::LayerWidget::visibilityChanged, this, &LayersWidget::onLayerWidgetVisibilityChanged_);
    connect(layerWidget, &impl_::LayerWidget::nameChanged, this, &LayersWidget::onLayerWidgetNameChanged_);
    connect(layerWidget, &impl_::LayerWidget::nameEditingFinished, this, &LayersWidget::onLayerWidgetNameEditingFinished_);
    connect(layerWidget, &impl_::LayerWidget::checkpoint, this, &LayersWidget::onLayerWidgetCheckpoint_);
}
