// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <QDialog>
#include "Settings.h"

#include <QDialogButtonBox>
#include <QDoubleSpinBox>

class SettingsDialog: public QDialog
{
    Q_OBJECT

public:
    SettingsDialog(QWidget * parent = 0);
    void go();

private slots:
    void cancel();
    void apply();
    void ok();

    void dialogButtonClicked(QAbstractButton * button);
    void writeApplicationSettings();
    void writeApplicationSettings(const Settings & settings);

signals:
    void preferencesChanged();

private:
    //void readApplicationPreferences();
    //void writeApplicationPreferences();

    Settings preferencesFromWidgetValues() const;
    void setWidgetValuesFromPreferences(const Settings & preferences);

    QDoubleSpinBox * edgeWidth_;


    QDialogButtonBox * dialogButtons_;
    Settings preferencesBak;

};

#endif
