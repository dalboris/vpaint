// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "EditCanvasSizeDialog.h"

#include "Scene.h"

#include <QDoubleSpinBox>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QFormLayout>

EditCanvasSizeDialog::EditCanvasSizeDialog(Scene * scene) :
    scene_(scene),
    ignoreSceneChanged_(false)
{
    // Form
    QFormLayout * formLayout = new QFormLayout();

    leftSpinBox_ = new QDoubleSpinBox();
    leftSpinBox_->setRange(-100000,100000);
    formLayout->addRow(tr("Left"), leftSpinBox_);

    topSpinBox_ = new QDoubleSpinBox();
    topSpinBox_->setRange(-100000,100000);
    formLayout->addRow(tr("Top"), topSpinBox_);

    widthSpinBox_ = new QDoubleSpinBox();
    widthSpinBox_->setRange(0,100000);
    formLayout->addRow(tr("Width"), widthSpinBox_);

    heightSpinBox_ = new QDoubleSpinBox();
    heightSpinBox_->setRange(0,100000);
    formLayout->addRow(tr("Height"), heightSpinBox_);

    // Ok/Cancel dialog buttons
    QDialogButtonBox * buttonBox = new QDialogButtonBox(
                QDialogButtonBox::Ok
                | QDialogButtonBox::Cancel);

    // Main layout
    QVBoxLayout * layout = new QVBoxLayout();
    layout->addLayout(formLayout);
    layout->addWidget(buttonBox);
    setLayout(layout);

    // Set initial widget values
    updateDialogFromScene();

    // Store initial values to allow Cancel
    backupCurentValues();

    // Create connections
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(scene_, SIGNAL(changed()), this, SLOT(updateDialogFromScene()));
    connect(topSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(updateSceneFromDialog()));
    connect(leftSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(updateSceneFromDialog()));
    connect(widthSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(updateSceneFromDialog()));
    connect(heightSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(updateSceneFromDialog()));
}

void EditCanvasSizeDialog::backupCurentValues()
{
    oldTop_ = scene_->top();
    oldLeft_ = scene_->left();
    oldWidth_ = scene_->width();
    oldHeight_ = scene_->height();

}

void EditCanvasSizeDialog::setVisible(bool visible)
{
    if(visible)
    {
        backupCurentValues();
    }

    QDialog::setVisible(visible);
}

Scene * EditCanvasSizeDialog::scene() const
{
    return scene_;
}

void EditCanvasSizeDialog::accept()
{
    QDialog::accept();
}

void EditCanvasSizeDialog::reject()
{
    scene_->setTop(oldTop_);
    scene_->setLeft(oldLeft_);
    scene_->setWidth(oldWidth_);
    scene_->setHeight(oldHeight_);

    QDialog::reject();
}

void EditCanvasSizeDialog::updateDialogFromScene()
{
    if(!ignoreSceneChanged_)
    {
        ignoreSceneChanged_ = true;

        topSpinBox_->setValue(scene()->top());
        leftSpinBox_->setValue(scene()->left());
        widthSpinBox_->setValue(scene()->width());
        heightSpinBox_->setValue(scene()->height());

        ignoreSceneChanged_ = false;
    }
}

void EditCanvasSizeDialog::updateSceneFromDialog()
{
    if(!ignoreSceneChanged_)
    {
        ignoreSceneChanged_ = true;

        scene()->setTop(topSpinBox_->value());
        scene()->setLeft(leftSpinBox_->value());
        scene()->setWidth(widthSpinBox_->value());
        scene()->setHeight(heightSpinBox_->value());

        ignoreSceneChanged_ = false;
    }
}
