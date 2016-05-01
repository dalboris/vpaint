// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "ViewMacOsX.h"

#include "ViewOld.h"

#include <QVBoxLayout>
#include <QPushButton>

ViewMacOsX::ViewMacOsX(SceneOld *scene, QWidget *parent) :
    QWidget(parent)
{
    view_ = new ViewOld(scene, this);

    QVBoxLayout * layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(view_->viewSettingsWidget()); // steal ownership
    layout->addWidget(view_);
    setLayout(layout);
}

ViewOld * ViewMacOsX::view() const
{
    return view_;
}
