// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef COLORSELECTOR_H
#define COLORSELECTOR_H

#include <QToolButton>
#include <QColor>

class ColorSelector : public QToolButton
{
    Q_OBJECT
public:
    explicit ColorSelector(const QColor & initialColor = Qt::black, QWidget *parent = 0);

    QColor color();
    void updateIcon();

public slots:
    void changeColor();

private:
    QColor color_;
};

#endif // COLORSELECTOR_H
