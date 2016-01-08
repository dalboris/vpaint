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
    background_(0),
    isUpdatingFromBackground_(false)
{
    // Layout
    QFormLayout * layout = new QFormLayout();
    setLayout(layout);

    // Color
    colorSelector_ = new ColorSelector(Qt::white);
    layout->addRow(tr("Color:"), colorSelector_);
    connect(colorSelector_, SIGNAL(colorChanged(Color)),
            this, SLOT(processColorSelectorColorChanged_(Color)));

    // Images
    imageLineEdit_ = new QLineEdit();
    imageBrowseButton_ = new QPushButton("...");
    imageBrowseButton_->setMaximumWidth(30);
    imageRefreshButton_ = new QPushButton("O"); // XXX set icon
    imageRefreshButton_->setMaximumWidth(30);
    QHBoxLayout * imagesLayout = new QHBoxLayout();
    imagesLayout->addWidget(imageLineEdit_);
    imagesLayout->addWidget(imageBrowseButton_);
    imagesLayout->addWidget(imageRefreshButton_);
    layout->addRow(tr("Image(s):"), imagesLayout);
    connect(imageLineEdit_, SIGNAL(textChanged(QString)),
            this, SLOT(processImageLineEditTextChanged_(QString)));
    connect(imageBrowseButton_, SIGNAL(clicked(bool)),
            this, SLOT(processImageBrowseButtonClicked_()));
    connect(imageRefreshButton_, SIGNAL(clicked(bool)),
            this, SLOT(processImageRefreshButtonClicked_()));

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
    connect(leftSpinBox_, SIGNAL(valueChanged(double)),
            this, SLOT(processLeftSpinBoxValueChanged_(double)));
    connect(topSpinBox_, SIGNAL(valueChanged(double)),
            this, SLOT(processTopSpinBoxValueChanged_(double)));

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
    connect(sizeComboBox_, SIGNAL(currentIndexChanged(int)),
            this, SLOT(processSizeComboBoxCurrentIndexChanged_(int)));
    connect(widthSpinBox_, SIGNAL(valueChanged(double)),
            this, SLOT(processWidthSpinBoxValueChanged_(double)));
    connect(heightSpinBox_, SIGNAL(valueChanged(double)),
            this, SLOT(processHeightSpinBoxValueChanged_(double)));

    // Repeat
    repeatComboBox_ = new QComboBox();
    repeatComboBox_->addItem(tr("No"));
    repeatComboBox_->addItem(tr("Horizontally"));
    repeatComboBox_->addItem(tr("Vertically"));
    repeatComboBox_->addItem(tr("Both"));
    repeatComboBox_->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));
    layout->addRow(tr("Repeat:"), repeatComboBox_);
    connect(repeatComboBox_, SIGNAL(currentIndexChanged(int)),
            this, SLOT(processRepeatComboBoxCurrentIndexChanged_(int)));

    // Opacity
    opacitySpinBox_ = new QDoubleSpinBox();
    opacitySpinBox_->setMaximumWidth(80);
    opacitySpinBox_->setMinimum(0);
    opacitySpinBox_->setMaximum(1);
    opacitySpinBox_->setSingleStep(0.1);
    opacitySpinBox_->setValue(1.0);
    layout->addRow(tr("Opacity:"), opacitySpinBox_);
    connect(opacitySpinBox_, SIGNAL(valueChanged(double)),
            this, SLOT(processOpacitySpinBoxValueChanged_(double)));

    // Hold
    holdCheckBox_ = new QCheckBox();
    holdCheckBox_->setChecked(true );
    layout->addRow(tr("Hold:"), holdCheckBox_);
    connect(holdCheckBox_, SIGNAL(toggled(bool)),
            this, SLOT(processHoldCheckBoxToggled_(bool)));

    // Set no background
    setBackground(0);
}

void BackgroundWidget::setBackground(Background * background)
{
    // Delete previous connections
    if (background_)
    {
        background_->disconnect(this);
    }

    // Store value
    background_ = background;

    // Disable all widgets if background_ == NULL
    bool areChildrenEnabled = background_ ? true : false;
    QList<QWidget*> children = findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
    foreach (QWidget * w, children)
        w->setEnabled(areChildrenEnabled);

    // Set widgets values from background values
    updateFromBackground_();

    // Create connections
    if (background_)
    {
        connect(background_, SIGNAL(changed()), this, SLOT(updateFromBackground_()));
    }
}

void BackgroundWidget::updateFromBackground_()
{
    if (background_)
    {
        // Set guard
        isUpdatingFromBackground_ = true;

        // Color
        colorSelector_->setColor(background_->color());


        // Image
        imageLineEdit_->setText(background_->imageUrl());

        // Position
        leftSpinBox_->setValue(background_->position()[0]);
        topSpinBox_->setValue(background_->position()[1]);

        // Size
        sizeComboBox_->setCurrentIndex((int) background_->sizeType());
        widthSpinBox_->setValue(background_->size()[0]);
        heightSpinBox_->setValue(background_->size()[1]);
        switch (background_->sizeType())
        {
        case Background::Cover:
            widthSpinBox_->hide();
            heightSpinBox_->hide();
            break;
        case Background::Manual:
            widthSpinBox_->show();
            heightSpinBox_->show();
            break;
        }

        // Repeat
        repeatComboBox_->setCurrentIndex((int) background_->repeatType());

        // Opacity
        opacitySpinBox_->setValue(background_->opacity());

        // Hold
        holdCheckBox_->setChecked(background_->hold());

        // Unset guard
        isUpdatingFromBackground_ = false;
    }
}

Background * BackgroundWidget::background() const
{
    return background_;
}

void BackgroundWidget::processColorSelectorColorChanged_(const Color & newColor)
{
    if (background_ && !isUpdatingFromBackground_)
    {
        background_->setColor(newColor);
    }
}

void BackgroundWidget::processImageLineEditTextChanged_(const QString & newText)
{
    if (background_ && !isUpdatingFromBackground_)
    {
        background_->setImageUrl(newText);
    }
}

void BackgroundWidget::processImageBrowseButtonClicked_()
{
    // todo
}

void BackgroundWidget::processImageRefreshButtonClicked_()
{
    // todo
}

void BackgroundWidget::processLeftSpinBoxValueChanged_(double newLeft)
{
    if (background_ && !isUpdatingFromBackground_)
    {
        double top = background_->position()[1];
        background_->setPosition(Eigen::Vector2d(newLeft, top));
    }
}

void BackgroundWidget::processTopSpinBoxValueChanged_(double newTop)
{
    if (background_ && !isUpdatingFromBackground_)
    {
        double left = background_->position()[0];
        background_->setPosition(Eigen::Vector2d(left, newTop));
    }
}

void BackgroundWidget::processSizeComboBoxCurrentIndexChanged_(int newSizeType)
{
    if (background_ && !isUpdatingFromBackground_)
    {
        background_->setSizeType(static_cast<Background::SizeType>(newSizeType));
    }
}

void BackgroundWidget::processWidthSpinBoxValueChanged_(double newWidth)
{
    if (background_ && !isUpdatingFromBackground_)
    {
        double height = background_->size()[1];
        background_->setSize(Eigen::Vector2d(newWidth, height));
    }
}

void BackgroundWidget::processHeightSpinBoxValueChanged_(double newHeight)
{
    if (background_ && !isUpdatingFromBackground_)
    {
        double width = background_->size()[0];
        background_->setSize(Eigen::Vector2d(width, newHeight));
    }
}

void BackgroundWidget::processRepeatComboBoxCurrentIndexChanged_(int newRepeatType)
{
    if (background_ && !isUpdatingFromBackground_)
    {
        background_->setRepeatType(static_cast<Background::RepeatType>(newRepeatType));
    }
}

void BackgroundWidget::processOpacitySpinBoxValueChanged_(double newOpacity)
{
    if (background_ && !isUpdatingFromBackground_)
    {
        background_->setOpacity(newOpacity);
    }
}

void BackgroundWidget::processHoldCheckBoxToggled_(bool newHold)
{
    if (background_ && !isUpdatingFromBackground_)
    {
        background_->setHold(newHold);
    }
}
