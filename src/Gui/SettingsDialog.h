// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

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
