// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef SPINBOX_H
#define SPINBOX_H

#include <QDoubleSpinBox>
#include <QLabel>

class SpinBox : public QWidget
{
    Q_OBJECT

public:
    explicit SpinBox(QWidget * parent = 0);
    
    // Caption
    QString caption() const;
    void setCaption(const QString & caption);

    // Value
    double value() const;

signals:
    void valueChanged(double d);

public slots:
    void setValue(double val);

private:
    QLabel * caption_;
    QDoubleSpinBox * spinBox_;
};

#endif // SPINBOX_H
