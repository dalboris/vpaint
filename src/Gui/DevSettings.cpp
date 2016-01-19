// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "DevSettings.h"

#include <QLabel>
#include <QtDebug>

DevSettings * DevSettings::s = 0;
DevSettings::DevSettings()
{
    s = this;
    layout_ = new QGridLayout();
    numWidgets_ = 0;

    addSection("Cutting");

    createCheckBox("reverse cut", false);
    createCheckBox("mobius cut", false);

    addSection("Inbetweening");

    createCheckBox("inverse direction", false);

    addSection("Rendering");

    createCheckBox("draw edge orientation", false);

    createSpinBox("num sub", 0, 10, 2);
    createDoubleSpinBox("ds", 0, 10, 2);

    setLayout(layout_);
}

bool DevSettings::getBool(const QString & name)
{
    if(!s || !s->checkBoxes_.contains(name))
    {
        qDebug() << "Settings: " << name << "not found";
        return false;
    }
    else
        return s->checkBoxes_[name]->isChecked();
}

int DevSettings::getInt(const QString & name)
{
    if(!s || !s->spinBoxes_.contains(name))
    {
        qDebug() << "Settings: " << name << "not found";
        return 0;
    }
    else
        return s->spinBoxes_[name]->value();
}

double DevSettings::getDouble(const QString & name)
{
    if(!s || !s->doubleSpinBoxes_.contains(name))
    {
        qDebug() << "Settings: " << name << "not found";
        return 0;
    }
    else
        return s->doubleSpinBoxes_[name]->value();
}

QSpinBox * DevSettings::createSpinBox(const QString & string, int min, int max, int value)
{
    QSpinBox * spinBox = new QSpinBox();
    spinBox->setMinimum(min);
    spinBox->setMaximum(max);
    spinBox->setValue(value);
    connect(spinBox, SIGNAL(valueChanged(int)),
          this, SIGNAL(changed()));

    addWidget(spinBox, string);
    spinBoxes_[string] = spinBox;
    
    return spinBox;
}

QDoubleSpinBox * DevSettings::createDoubleSpinBox(const QString & string, double min, double max, double value)
{
    QDoubleSpinBox * spinBox = new QDoubleSpinBox();
    spinBox->setMinimum(min);
    spinBox->setMaximum(max);
    spinBox->setValue(value);
    spinBox->setSingleStep(0.1);
    connect(spinBox, SIGNAL(valueChanged(double)),
          this, SIGNAL(changed()));

    addWidget(spinBox, string);
    doubleSpinBoxes_[string] = spinBox;
    
    return spinBox;
}

    
QCheckBox * DevSettings::createCheckBox(const QString & string, bool checked)
{
    QCheckBox * checkBox = new QCheckBox();
    checkBox->setChecked(checked);
    connect(checkBox, SIGNAL(toggled(bool)),
          this, SIGNAL(changed()));
    
    addWidget(checkBox, string);
    checkBoxes_[string] = checkBox;
    
    return checkBox;
}

void DevSettings::addWidget(QWidget *widget, const QString & string)
{
    QLabel *label = new QLabel(string);
    layout_->addWidget(widget, numWidgets_, 0);
    layout_->addWidget(label, numWidgets_, 1);
    numWidgets_++;
}

void DevSettings::addSection(const QString & string)
{
    QString str;
    str.append("<b>");
    str.append(string);
    str.append("</b>");
    QLabel *label = new QLabel(str);
    label->setAlignment(Qt::AlignHCenter/*Left*/ | Qt::AlignBottom);
    if(numWidgets_!=0)
        label->setMinimumHeight(40);
    layout_->addWidget(label, numWidgets_, 0, 1, 2);
    numWidgets_++;
}
