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
    VAC * vac = global()->mainWindow()->scene()->activeVAC();
    if(vac)
    {
        for(Cell * c: vac->selectedCells())
        {
            text += QString::number(c->id());
            text += " ";
        }
    }

    labelSelected_->setText(text);
}
