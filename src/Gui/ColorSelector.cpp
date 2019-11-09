// Copyright (C) 2012-2019 The VPaint Developers
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
    QPixmap checkerboard(":/images/checkerboard.png"), pix(pixSize);
    //checkerboard.brush(color_);
    QPainter painter(&pix);
    QRect r(0,0,w-1,h-1);
    painter.fillRect(r, QBrush(checkerboard));
    painter.fillRect(r, QBrush(color_));

    // Border
    painter.setPen(QPen(Qt::black));
    painter.drawRect(r);

    // Text
    painter.setPen(QPen(textColor));
    painter.setFont(QFont("Arial"));
    painter.drawText(0, 0, w, h, (Qt::AlignHCenter |  Qt::AlignVCenter), text());

    // Set pixmap as tool icon
    setIcon(pix);
}

