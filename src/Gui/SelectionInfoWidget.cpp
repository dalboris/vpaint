// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "SelectionInfoWidget.h"

#include <QGridLayout>
#include <QPushButton>
#include "Global.h"
#include "MainWindow.h"
#include "Scene.h"
#include "VectorAnimationComplex/VAC.h"

SelectionInfoWidget::SelectionInfoWidget(QWidget *parent) :
    QWidget(parent)
{
    setWindowTitle("Selection Info");
    mainLayout_ = new QGridLayout();
    labelSelected_ = new QLabel();
    mainLayout_->addWidget(labelSelected_,0,0);
    setLayout(mainLayout_);

    updateInfo();
}


void SelectionInfoWidget::updateInfo()
{
    QString text;

    using namespace VectorAnimationComplex;
    VAC * vac = global()->mainWindow()->scene()->vectorAnimationComplex();
    if(vac)
    {
        foreach(Cell * c, vac->selectedCells())
        {
            text += QString::number(c->id());
            text += " ";
        }
    }

    labelSelected_->setText(text);
}
