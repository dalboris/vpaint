// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "SpinBox.h"

#include <QVBoxLayout>

SpinBox::SpinBox(QWidget *parent) :
    QWidget(parent),
    caption_(0),
    spinBox_(0)
{
    // Caption
    caption_ = new QLabel();
    QFont labelFont = font();
    labelFont.setPixelSize(11);
    caption_->setFont(labelFont);
    caption_->setAlignment(Qt::AlignCenter);

    // Spin box
    spinBox_ = new QDoubleSpinBox();
    spinBox_->setRange(0.0, 999.99);

    // Layout
    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget(caption_);
    layout->addWidget(spinBox_);
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);
    setLayout(layout);
    setFixedHeight(40);

    // Signal forwarding
    connect(spinBox_, SIGNAL(valueChanged(double)), this, SIGNAL(valueChanged(double)));
}

QString SpinBox::caption() const
{
    return caption_->text();
}

void SpinBox::setCaption(const QString & caption)
{
    caption_->setText(caption);
}

double SpinBox::value() const
{
    return spinBox_->value();
}

void SpinBox::setValue(double val)
{
    spinBox_->setValue(val);
}
