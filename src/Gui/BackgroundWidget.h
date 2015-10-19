// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef BACKGROUNDWIDGET_H
#define BACKGROUNDWIDGET_H

#include <QWidget>

#include "Background.h"

class BackgroundWidget: public QWidget
{
public:
    BackgroundWidget(QWidget * parent = 0);

    const Background & background() const;

private:
    Background background_;
};

#endif // BACKGROUNDWIDGET_H
