// Copyright (C) 2012-2019 The VPaint Developers.
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

#ifndef COLORSELECTOR_H
#define COLORSELECTOR_H

#include <QToolButton>

#include "Color.h"
#include "VAC/vpaint_global.h"

class Q_VPAINT_EXPORT ColorSelector : public QToolButton
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
