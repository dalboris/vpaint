// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "ViewMacOsX.h"

#include "View.h"

#include <QVBoxLayout>
#include <QPushButton>

ViewMacOsX::ViewMacOsX(Scene *scene, QWidget *parent) :
    QWidget(parent)
{
    view_ = new View(scene, this);

    QVBoxLayout * layout = new QVBoxLayout();
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(view_->viewSettingsWidget()); // steal ownership
    layout->addWidget(view_);
    setLayout(layout);
}

View * ViewMacOsX::view() const
{
    return view_;
}
