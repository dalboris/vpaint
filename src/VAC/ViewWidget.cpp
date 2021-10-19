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

#include "ViewWidget.h"
#include "View.h"
#include "ViewSettings.h"

#include <QVBoxLayout>

ViewWidget::ViewWidget(VPaint::Scene *scene, QWidget *parent) :
    QWidget(parent)
{
    view_ = new View(scene, this);

    QVBoxLayout * layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(view_);
    setLayout(layout);
}

View * ViewWidget::view() const
{
    return view_;
}

ViewSettingsWidget * ViewWidget::viewSettingsWidget() const
{
    return view()->viewSettingsWidget();
}
