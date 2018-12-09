// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "ViewWidget.h"
#include "View.h"
#include "ViewSettings.h"

#include <QVBoxLayout>

ViewWidget::ViewWidget(Scene *scene, QWidget *parent) :
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
