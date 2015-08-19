// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef DEV_SETTINGS_H
#define DEV_SETTINGS_H

// This class is intended for development only, to quickly try out
// different parameters and find out what are the best.
//
// For actual deployment, those dev settings should either be moved to
// "settings" (i.e., non-dev) and make it user-controlable, or be
// moved directly to global.

#include <QWidget>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QString>
#include <QMap>


class DevSettings : public QWidget
{
    Q_OBJECT
    
public:
    DevSettings();
    static bool getBool(const QString & name);
    static int getInt(const QString & name);
    static double getDouble(const QString & name);
    static DevSettings * instance()
        {return s;}

signals:
    void changed();

private:
    static DevSettings *s;

    // bool values
    QMap<QString,QCheckBox*> checkBoxes_;
    QCheckBox * createCheckBox(const QString & string, bool checked);

    // int values
    QMap<QString,QSpinBox*> spinBoxes_;
    QSpinBox * createSpinBox(const QString & string, int min, int max, int value);

    // double values
    QMap<QString,QDoubleSpinBox*> doubleSpinBoxes_;
    QDoubleSpinBox * createDoubleSpinBox(const QString & string, double min, double max, double value);

    // layout
    void addSection(const QString & string);
    void addWidget(QWidget *widget, const QString & string);
    QGridLayout *layout_;
    int numWidgets_;
};

#endif
