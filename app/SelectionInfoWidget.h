// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef SELECTIONINFOWIDGET_H
#define SELECTIONINFOWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QLabel>

class Scene;

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
