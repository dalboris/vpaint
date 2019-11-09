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
