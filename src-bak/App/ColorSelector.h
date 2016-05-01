// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef COLORSELECTOR_H
#define COLORSELECTOR_H

#include <QToolButton>

#include "Color.h"

class ColorSelector : public QToolButton
{
    Q_OBJECT

public:
    explicit ColorSelector(const Color & initialColor = Qt::black, QWidget *parent = 0);

    // Get/Set color
    Color color() const;
    void setColor(const Color & newColor);

    // Set icon of QToolButton to match stored color.
    //
    // If clients change the icon size via setIconSize(), then they
    // must call updateIcon() afterwards. Ideally setIconSize() should be
    // overriden call this automatically, but unfortunately  setIconSize()
    // isn't a virtual method.
    void updateIcon();

signals:
    void colorChanged(const Color & newColor);

private slots:
    void processClick_();

private:
    // Stored color
    Color color_;
};

#endif // COLORSELECTOR_H
