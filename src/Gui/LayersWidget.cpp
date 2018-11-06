// Copyright (C) 2012-2018 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

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

namespace
{
const QColor activeLayerBackgroundColor = QColor::fromRgb(128, 190, 230);
const QColor inactiveLayerBackgroundColor = QColor::fromRgb(255, 255, 255);
const QColor noLayerBackgroundColor = QColor::fromRgb(255, 255, 255);
} // namespace

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

    nameLabel_ = new QLabel();
    nameLabel_->setMinimumHeight(30);

    nameLineEdit_ = new QLineEdit();
    nameLineEdit_->setMinimumHeight(30);
    nameLineEdit_->hide();
    connect(nameLineEdit_, &QLineEdit::editingFinished, this, &LayerWidget::onNameEditingFinished_);

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

void LayerWidget::setName(const QString& newName)
{
    // Abort editing if any
    if (nameLineEdit_->isVisible())
    {
        nameLineEdit_->hide();
        nameLabel_->show();
    }

    // Set new name if different form current name
    if (newName != name())
    {
        nameLabel_->setText(newName);
        emit nameChanged(index());
    }
}

void LayerWidget::enterNameEditingMode()
{
    nameLineEdit_->setText(name());
    nameLabel_->hide();
    nameLineEdit_->show();
    nameLineEdit_->selectAll();
    nameLineEdit_->setFocus();
}

void LayerWidget::mousePressEvent(QMouseEvent*)
{
    setActive(true);
}

void LayerWidget::mouseDoubleClickEvent(QMouseEvent*)
{
    enterNameEditingMode();
}

void LayerWidget::onVisibilityCheckBoxStateChanged_(int)
{
    emit visibilityChanged(index());
}

void LayerWidget::onNameEditingFinished_()
{
    setName(nameLineEdit_->text());
}

void LayerWidget::updateBackground_()
{
    QColor color = isActive() ? activeLayerBackgroundColor : inactiveLayerBackgroundColor;

    QPalette p = palette();
    p.setColor(QPalette::Background, color);
    setPalette(p);
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
    QPalette p = scrollArea->palette();
    p.setColor(QPalette::Background, noLayerBackgroundColor);
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
            activeLayerWidget_->enterNameEditingMode();
        }
    }
}

void LayersWidget::onDeleteLayerClicked_()
{
    scene()->destroyActiveLayer();
}

void LayersWidget::onMoveLayerUpClicked_()
{
    scene()->moveActiveLayerUp();
}

void LayersWidget::onMoveLayerDownClicked_()
{
    scene()->moveActiveLayerDown();
}

void LayersWidget::onSceneLayerAttributesChanged_()
{
    updateUiFromScene_();
}

void LayersWidget::updateUiFromScene_()
{
    // Show as many existing LayerWidgets as necessary
    int numLayers = scene()->numLayers();
    int numLayerWidgets = layerWidgets_.size();
    int newNumVisibleLayerWidgets = std::min(numLayers, numLayerWidgets);
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
    impl_::LayerWidget * layerWidget = new impl_::LayerWidget(layerWidgets_.size());
    ++numVisibleLayerWidgets_;
    layerWidgets_.push_back(layerWidget);
    layerListLayout_->addWidget(layerWidget);
    connect(layerWidget, &impl_::LayerWidget::activated, this, &LayersWidget::onLayerWidgetActivated_);
    connect(layerWidget, &impl_::LayerWidget::visibilityChanged, this, &LayersWidget::onLayerWidgetVisibilityChanged_);
    connect(layerWidget, &impl_::LayerWidget::nameChanged, this, &LayersWidget::onLayerWidgetNameChanged_);
}
