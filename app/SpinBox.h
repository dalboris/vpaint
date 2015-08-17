// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

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
