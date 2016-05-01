// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef SELECTIONINFOWIDGET_H
#define SELECTIONINFOWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QLabel>

class SceneOld;

class SelectionInfoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SelectionInfoWidget(QWidget *parent = 0);

signals:

public slots:
    void updateInfo();

private:
    QLabel * labelSelected_;
    QGridLayout * mainLayout_;
};

#endif // SELECTIONINFOWIDGET_H
