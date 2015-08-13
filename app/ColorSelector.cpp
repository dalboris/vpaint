// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "ColorSelector.h"
#include <QColorDialog>
#include <QPainter>

ColorSelector::ColorSelector(const QColor & initialColor, QWidget *parent) :
    QToolButton(parent),
    color_(initialColor)
{
    connect(this, SIGNAL(clicked()), this, SLOT(changeColor()));
    updateIcon();
    setFocusPolicy(Qt::NoFocus);
}

QColor ColorSelector::color()
{
    return color_;
}

void ColorSelector::changeColor()
{
    QColor c = QColorDialog::getColor(
                color_, 0, tr("select the color"),
                QColorDialog::ShowAlphaChannel);
    if(c.isValid())
        color_ = c;
    updateIcon();
}

#include <QtDebug>

void ColorSelector::updateIcon()
{
    // Compute text color depending on background color
    QColor textColor(0,0,0);
    if(color_.lightnessF() < 0.5)
        textColor = QColor(255,255,255);

    // Get icon size
    QSize pixSize = iconSize();
    int w = pixSize.width();
    int h = pixSize.height();

    // Create icon
    QPixmap pix(pixSize);
    pix.fill(color_);

    QPainter painter( &pix );

    // Border
    painter.setPen(QPen(Qt::black));
    painter.drawRect(0,0,w-1,h-1);

    // Text
    painter.setPen(QPen(textColor));
    painter.setFont(QFont("Arial"));
    painter.drawText( 0, 0, w, h, (Qt::AlignHCenter |  Qt::AlignVCenter), text() );

    // Set pixmap as tool icon
    setIcon(pix);

    // Apply new colors to button
    /*
        QString qss = QString("QToolButton { background-color: %1; color: %2; font-weight: %3; }")
                .arg(color_.name())
                .arg(textColor.name())
                .arg("normal");
        setStyleSheet(qss);
        */
}
