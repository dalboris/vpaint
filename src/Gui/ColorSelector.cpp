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

#include "ColorSelector.h"
#include <QColorDialog>
#include <QPainter>

ColorSelector::ColorSelector(const Color & initialColor, QWidget *parent) :
    QToolButton(parent),
    color_(initialColor)
{
    connect(this, SIGNAL(clicked()), this, SLOT(processClick_()));
    updateIcon();
    setFocusPolicy(Qt::NoFocus);
}

Color ColorSelector::color() const
{
    return color_;
}

void ColorSelector::setColor(const Color & newColor)
{
    if (newColor.isValid())
    {
        color_ = newColor;
        updateIcon();
        emit colorChanged(color_);
    }
}

void ColorSelector::processClick_()
{
    Color c = QColorDialog::getColor(
                color_,
                0,
                tr("select the color"),
                QColorDialog::ShowAlphaChannel);

    setColor(c);
}

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

    QPainter painter(&pix);

    // Border
    painter.setPen(QPen(Qt::black));
    painter.drawRect(0,0,w-1,h-1);

    // Text
    painter.setPen(QPen(textColor));
    painter.setFont(QFont("Arial"));
    painter.drawText(0, 0, w, h, (Qt::AlignHCenter |  Qt::AlignVCenter), text());

    // Set pixmap as tool icon
    setIcon(pix);
}

