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

#include "ExportPngDialog.h"

#include "Scene.h"

#include <cmath>

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

ExportPngDialog::ExportPngDialog(Scene * scene) :
    scene_(scene),
    ignoreWidthHeightChanged_(false)
{
    // General dialog properties
    setWindowTitle(tr("Export as PNG"));
    setMinimumWidth(280);

    // PNG Size
    QGroupBox * pngSizeGroupBox = new QGroupBox(tr("PNG Image Size"));
    QVBoxLayout * pngSizeLayout = new QVBoxLayout();
    pngSizeGroupBox->setLayout(pngSizeLayout);

    QFormLayout * pngWidthHeightLayout = new QFormLayout();
    pngSizeLayout->addLayout(pngWidthHeightLayout);

    pngWidthSpinBox_ = new QSpinBox();
    pngWidthSpinBox_->setRange(1,100000);
    pngWidthSpinBox_->setValue(1280);
    pngWidthSpinBox_->setMaximumWidth(60);
    pngWidthHeightLayout->addRow(tr("Width:"), pngWidthSpinBox_);

    pngHeightSpinBox_ = new QSpinBox();
    pngHeightSpinBox_->setRange(1,100000);
    pngHeightSpinBox_->setValue(720);
    pngHeightSpinBox_->setMaximumWidth(60);
    pngWidthHeightLayout->addRow(tr("Height:"), pngHeightSpinBox_);

    preserveAspectRatioCheckBox_ = new QCheckBox(tr("Preserve canvas aspect ratio"));
    preserveAspectRatioCheckBox_->setChecked(true);
    pngSizeLayout->addWidget(preserveAspectRatioCheckBox_);

    // Animation
    QGroupBox * animationGroupBox = new QGroupBox(tr("Animation"));
    QVBoxLayout * animationLayout = new QVBoxLayout();
    animationGroupBox->setLayout(animationLayout);

    exportSequenceCheckBox_ = new QCheckBox(tr("Export animation as image sequence"));
    exportSequenceCheckBox_->setChecked(false);
    animationLayout->addWidget(exportSequenceCheckBox_);

    // Rendering
    QGroupBox * renderingGroupBox = new QGroupBox(tr("Rendering"));
    QVBoxLayout * renderingLayout = new QVBoxLayout();
    renderingGroupBox->setLayout(renderingLayout);

    useViewSettings_ = new QCheckBox(tr("Use view settings"));
    useViewSettings_->setChecked(false);
    renderingLayout->addWidget(useViewSettings_);

    motionBlurCheckBox_ = new QCheckBox(tr("Motion blur"));
    motionBlurCheckBox_->setChecked(false);
    renderingLayout->addWidget(motionBlurCheckBox_);

    motionBlurOptionsLayout_ = new QFormLayout();
    renderingLayout->addLayout(motionBlurOptionsLayout_);

    motionBlurNumSamplesSpinBox_ = new QSpinBox();
    motionBlurNumSamplesSpinBox_->setRange(1,100000);
    motionBlurNumSamplesSpinBox_->setValue(16);
    motionBlurNumSamplesSpinBox_->setMaximumWidth(60);
    motionBlurOptionsLayout_->addRow(tr("        number of samples:"), motionBlurNumSamplesSpinBox_);

    // Hide motion blur options until checked
    processMotionBlurChanged_(motionBlurCheckBox_->isChecked());

    // Export/Cancel dialog buttons
    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel);
    buttonBox->addButton(tr("Export"), QDialogButtonBox::AcceptRole);

    // Main layout
    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget(pngSizeGroupBox);
    layout->addWidget(animationGroupBox);
    layout->addWidget(renderingGroupBox);
    layout->addStretch();
    layout->addWidget(buttonBox);
    setLayout(layout);

    // Set initial widget values
    updateDialogFromScene();

    // Create connections
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    connect(scene_, SIGNAL(changed()), this, SLOT(updateDialogFromScene()));

    connect(pngWidthSpinBox_, SIGNAL(valueChanged(int)), this, SLOT(processPngWidthChanged_(int)));
    connect(pngHeightSpinBox_, SIGNAL(valueChanged(int)), this, SLOT(processPngHeightChanged_(int)));
    connect(preserveAspectRatioCheckBox_, SIGNAL(toggled(bool)), this, SLOT(processPreserveAspectRatioChanged_(bool)));
    connect(motionBlurCheckBox_, SIGNAL(toggled(bool)), this, SLOT(processMotionBlurChanged_(bool)));
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

bool ExportPngDialog::useViewSettings() const
{
    return useViewSettings_->isChecked();
}

bool ExportPngDialog::motionBlur() const
{
    return motionBlurCheckBox_->isChecked();
}

int ExportPngDialog::motionBlurNumSamples() const
{
    return motionBlurNumSamplesSpinBox_->value();
}

void ExportPngDialog::setPngWidthForHeight_()
{
    int sw = scene()->width();
    int sh = scene()->height();
    ignoreWidthHeightChanged_ = true; // prevent infinite recursion with pngWidthChanged()
    if(sh > 0)
        pngWidthSpinBox_->setValue(sw * pngHeight() / sh);
    ignoreWidthHeightChanged_ = false;
}

void ExportPngDialog::setPngHeightForWidth_()
{
    int sw = scene()->width();
    int sh = scene()->height();
    ignoreWidthHeightChanged_ = true; // prevent infinite recursion with pngHeightChanged()
    if(sw > 0)
        pngHeightSpinBox_->setValue(sh * pngWidth() / sw);
    ignoreWidthHeightChanged_ = false;
}

void ExportPngDialog::enforcePngAspectRatio_()
{
    if(preserveAspectRatio())
    {
        if(pngWidth() > pngHeight())
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

void ExportPngDialog::processMotionBlurChanged_(bool b)
{
    // Iterate over all widgets in motionBlurOptionsLayout_
    // and change their visibility.
    int n = motionBlurOptionsLayout_->count();
    for (int i = 0; i < n; ++i) {
        QLayoutItem * item = motionBlurOptionsLayout_->itemAt(i);
        if (item) {
             QWidget * widget = item->widget();
             if (widget) {
                 widget->setVisible(b);
             }
        }
    }
}

void ExportPngDialog::setVisible(bool visible)
{
    if(visible)
    {
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
    QDialog::reject();
}

void ExportPngDialog::updateDialogFromScene()
{
    ignoreWidthHeightChanged_ = true;
    pngWidthSpinBox_->setValue(scene()->width());
    pngHeightSpinBox_->setValue(scene()->height());
    ignoreWidthHeightChanged_ = false;
}
