// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "BackgroundWidget.h"

BackgroundWidget::BackgroundWidget(QWidget * parent) :
    QWidget(parent),
    background_()
{

}

const Background & BackgroundWidget::background() const
{
    return background_;
}
