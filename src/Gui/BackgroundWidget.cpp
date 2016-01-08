// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "BackgroundWidget.h"

#include "Background.h"
#include "ColorSelector.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QComboBox>

BackgroundWidget::BackgroundWidget(QWidget * parent) :
    QWidget(parent),
    background_(0)
{
    // Layout
    QFormLayout * layout = new QFormLayout();
    setLayout(layout);

    // Color
    colorSelector_ = new ColorSelector(Qt::white);
    layout->addRow(tr("Color:"), colorSelector_);

    // Images
    imagesTextEdit_ = new QLineEdit();
    imagesButton_ = new QPushButton("...");
    imagesButton_->setMaximumWidth(30);
    QHBoxLayout * imagesLayout = new QHBoxLayout();
    imagesLayout->addWidget(imagesTextEdit_);
    imagesLayout->addWidget(imagesButton_);
    layout->addRow(tr("Image(s):"), imagesLayout);

    // Position
    leftSpinBox_ = new QDoubleSpinBox();
    leftSpinBox_->setMaximumWidth(80);
    leftSpinBox_->setMinimum(-1e6);
    leftSpinBox_->setMaximum(1e6);
    topSpinBox_ = new QDoubleSpinBox();
    topSpinBox_->setMaximumWidth(80);
    topSpinBox_->setMinimum(-1e6);
    topSpinBox_->setMaximum(1e6);
    QHBoxLayout * positionLayout = new QHBoxLayout();
    positionLayout->addWidget(leftSpinBox_);
    positionLayout->addWidget(topSpinBox_);
    layout->addRow(tr("Position:"), positionLayout);

    // Size
    sizeComboBox_ = new QComboBox();
    sizeComboBox_->addItem(tr("Fit to canvas"));
    sizeComboBox_->addItem(tr("Manual"));
    sizeComboBox_->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));
    widthSpinBox_ = new QDoubleSpinBox();
    widthSpinBox_->setMaximumWidth(80);
    widthSpinBox_->setMinimum(-1e6);
    widthSpinBox_->setMaximum(1e6);
    widthSpinBox_->setValue(1280);
    heightSpinBox_ = new QDoubleSpinBox();
    heightSpinBox_->setMaximumWidth(80);
    heightSpinBox_->setMinimum(-1e6);
    heightSpinBox_->setMaximum(1e6);
    heightSpinBox_->setValue(720);
    QGridLayout * sizeLayout = new QGridLayout();
    sizeLayout->addWidget(sizeComboBox_, 0, 0, 1, 2);
    sizeLayout->addWidget(widthSpinBox_, 1, 0);
    sizeLayout->addWidget(heightSpinBox_, 1, 1);
    layout->addRow(tr("Size:"), sizeLayout);

    // Repeat
    repeatComboBox_ = new QComboBox();
    repeatComboBox_->addItem(tr("No"));
    repeatComboBox_->addItem(tr("Horizontally"));
    repeatComboBox_->addItem(tr("Vertically"));
    repeatComboBox_->addItem(tr("Both"));
    repeatComboBox_->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));
    layout->addRow(tr("Repeat:"), repeatComboBox_);

    // Opacity
    opacitySpinBox_ = new QDoubleSpinBox();
    opacitySpinBox_->setMaximumWidth(80);
    opacitySpinBox_->setMinimum(0);
    opacitySpinBox_->setMaximum(1);
    opacitySpinBox_->setSingleStep(0.1);
    opacitySpinBox_->setValue(1.0);
    layout->addRow(tr("Opacity:"), opacitySpinBox_);

    // Hold
    holdCheckBox_ = new QCheckBox();
    holdCheckBox_->setChecked(true );
    layout->addRow(tr("Hold:"), holdCheckBox_);

    // Set background
    setBackground(0);
}

void BackgroundWidget::setBackground(Background * background)
{
    // Store value
    background_ = background;

    // Disable all widgets if background_ == NULL
    bool areChildrenEnabled = background_ ? true : false;
    QList<QWidget*> children = findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
    foreach (QWidget * w, children)
        w->setEnabled(areChildrenEnabled);

    // Set widgets values from background values
    if (background_)
    {
        // todo
        // colorSelector_->setColor(...);
    }
}

Background * BackgroundWidget::background() const
{
    return background_;
}
