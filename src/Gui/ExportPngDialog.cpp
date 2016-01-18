// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "ExportPngDialog.h"

#include "Scene.h"

#include <cmath>

#include <QDoubleSpinBox>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QCheckBox>
#include <QLabel>

ExportPngDialog::ExportPngDialog(Scene * scene) :
    scene_(scene),
    ignoreSceneChanged_(false),
    ignoreWidthHeightChanged_(false)
{
    // Set window title
    setWindowTitle(tr("Export as PNG"));

    // Edit Canvas
    QFormLayout * formLayoutCanvas = new QFormLayout();

    leftSpinBox_ = new QDoubleSpinBox();
    leftSpinBox_->setRange(-100000,100000);
    formLayoutCanvas->addRow(tr("Left"), leftSpinBox_);

    topSpinBox_ = new QDoubleSpinBox();
    topSpinBox_->setRange(-100000,100000);
    formLayoutCanvas->addRow(tr("Top"), topSpinBox_);

    widthSpinBox_ = new QDoubleSpinBox();
    widthSpinBox_->setRange(0,100000);
    formLayoutCanvas->addRow(tr("Width"), widthSpinBox_);

    heightSpinBox_ = new QDoubleSpinBox();
    heightSpinBox_->setRange(0,100000);
    formLayoutCanvas->addRow(tr("Height"), heightSpinBox_);

    // PNG Export options
    QFormLayout * formLayoutPng = new QFormLayout();

    pngWidthSpinBox_ = new QSpinBox();
    pngWidthSpinBox_->setRange(1,100000);
    pngWidthSpinBox_->setValue(1280);
    formLayoutPng->addRow(tr("PNG Width"), pngWidthSpinBox_);

    pngHeightSpinBox_ = new QSpinBox();
    pngHeightSpinBox_->setRange(1,100000);
    pngHeightSpinBox_->setValue(720);
    formLayoutPng->addRow(tr("PNG Height"), pngHeightSpinBox_);

    preserveAspectRatioCheckBox_ = new QCheckBox();
    preserveAspectRatioCheckBox_->setChecked(true);
    formLayoutPng->addRow(tr("Preserve Aspect Ratio"), preserveAspectRatioCheckBox_);

    exportSequenceCheckBox_ = new QCheckBox();
    exportSequenceCheckBox_->setChecked(false);
    formLayoutPng->addRow(tr("Export Sequence"), exportSequenceCheckBox_);

    // Export/Cancel dialog buttons
    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel);
    buttonBox->addButton(tr("Export"), QDialogButtonBox::AcceptRole);

    // Main layout
    QVBoxLayout * layout = new QVBoxLayout();
    //layout->addWidget(new QLabel(tr("<b>Canvas Size</b>")));
    //layout->addLayout(formLayoutCanvas);
    //layout->addWidget(new QLabel(tr("<b>PNG Export Options</b>")));
    layout->addLayout(formLayoutPng);
    layout->addStretch();
    layout->addWidget(buttonBox);
    setLayout(layout);

    // Set initial widget values
    updateDialogFromScene();

    // Store initial values to allow Cancel
    backupCurrentCanvasSize_();

    // Create connections
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    connect(scene_, SIGNAL(changed()), this, SLOT(updateDialogFromScene()));

    connect(topSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(processCanvasSizeChanged_()));
    connect(leftSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(processCanvasSizeChanged_()));
    connect(widthSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(processCanvasSizeChanged_()));
    connect(heightSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(processCanvasSizeChanged_()));

    connect(pngWidthSpinBox_, SIGNAL(valueChanged(int)), this, SLOT(processPngWidthChanged_(int)));
    connect(pngHeightSpinBox_, SIGNAL(valueChanged(int)), this, SLOT(processPngHeightChanged_(int)));
    connect(preserveAspectRatioCheckBox_, SIGNAL(toggled(bool)), this, SLOT(processPreserveAspectRatioChanged_(bool)));
}

double ExportPngDialog::left() const
{
    return leftSpinBox_->value();
}

double ExportPngDialog::top() const
{
    return topSpinBox_->value();
}

double ExportPngDialog::width() const
{
    return widthSpinBox_->value();
}

double ExportPngDialog::height() const
{
    return heightSpinBox_->value();
}

int ExportPngDialog::pngWidth() const
{
    return pngWidthSpinBox_->value();
}

int ExportPngDialog::pngHeight() const
{
    return pngHeightSpinBox_->value();
}

bool ExportPngDialog::preserveAspectRatio() const
{
    return preserveAspectRatioCheckBox_->isChecked();
}

bool ExportPngDialog::exportSequence() const
{
    return exportSequenceCheckBox_->isChecked();
}

void ExportPngDialog::setPngWidthForHeight_()
{
    ignoreWidthHeightChanged_ = true; // prevent infinite recursion with pngWidthChanged()
    if(height() > 0)
        pngWidthSpinBox_->setValue(floor(0.5+width() * pngHeight() / height()));
    ignoreWidthHeightChanged_ = false;
}

void ExportPngDialog::setPngHeightForWidth_()
{
    ignoreWidthHeightChanged_ = true; // prevent infinite recursion with pngHeightChanged()
    if(width() > 0)
        pngHeightSpinBox_->setValue(floor(0.5+height() * pngWidth() / width()));
    ignoreWidthHeightChanged_ = false;
}

void ExportPngDialog::enforcePngAspectRatio_()
{
    if(preserveAspectRatio())
    {
        if(width() > height())
            setPngHeightForWidth_();
        else
            setPngWidthForHeight_();
    }

}

void ExportPngDialog::processPngWidthChanged_(int )
{
    if(!ignoreWidthHeightChanged_ && preserveAspectRatio())
    {
        setPngHeightForWidth_();
    }
}

void ExportPngDialog::processPngHeightChanged_(int )
{
    if(!ignoreWidthHeightChanged_ && preserveAspectRatio())
    {
            setPngWidthForHeight_();
    }
}

void ExportPngDialog::processPreserveAspectRatioChanged_(bool )
{
    enforcePngAspectRatio_();
}

void ExportPngDialog::backupCurrentCanvasSize_()
{
    oldTop_ = scene_->top();
    oldLeft_ = scene_->left();
    oldWidth_ = scene_->width();
    oldHeight_ = scene_->height();
}

void ExportPngDialog::setVisible(bool visible)
{
    if(visible)
    {
        backupCurrentCanvasSize_();
        enforcePngAspectRatio_();
    }

    QDialog::setVisible(visible);
}

Scene * ExportPngDialog::scene() const
{
    return scene_;
}

void ExportPngDialog::accept()
{
    QDialog::accept();
}

void ExportPngDialog::reject()
{
    scene_->setTop(oldTop_);
    scene_->setLeft(oldLeft_);
    scene_->setWidth(oldWidth_);
    scene_->setHeight(oldHeight_);

    QDialog::reject();
}

void ExportPngDialog::processCanvasSizeChanged_()
{
    enforcePngAspectRatio_();
    updateSceneFromDialog();
}

void ExportPngDialog::updateDialogFromScene()
{
    if(!ignoreSceneChanged_)
    {
        topSpinBox_->setValue(scene()->top());
        leftSpinBox_->setValue(scene()->left());
        widthSpinBox_->setValue(scene()->width());
        heightSpinBox_->setValue(scene()->height());
    }
}

void ExportPngDialog::updateSceneFromDialog()
{
    ignoreSceneChanged_ = true;

    scene()->setTop(topSpinBox_->value());
    scene()->setLeft(leftSpinBox_->value());
    scene()->setWidth(widthSpinBox_->value());
    scene()->setHeight(heightSpinBox_->value());

    ignoreSceneChanged_ = false;
}
