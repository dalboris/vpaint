// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "SettingsDialog.h"
#include "Global.h"

#include <QVBoxLayout>

//////////////////////////////////////////////////////////////////
//////////////      CREATE WIDGETS AND LAYOUT       //////////////
//////////////////////////////////////////////////////////////////

SettingsDialog::SettingsDialog(QWidget * parent) :
    QDialog(parent)
{
    // Create all widgets
    edgeWidth_ = new QDoubleSpinBox();
    edgeWidth_->setRange(0.0, 999.99);


    // setup layout
    QVBoxLayout * mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(edgeWidth_);

    // Preference dialog buttons
    dialogButtons_ = new QDialogButtonBox(QDialogButtonBox::Ok |
                                          QDialogButtonBox::Cancel |
                                          QDialogButtonBox::Apply);
    connect(dialogButtons_,SIGNAL(clicked(QAbstractButton*)),this,SLOT(dialogButtonClicked(QAbstractButton*)));
    mainLayout->addWidget(dialogButtons_);

}


//////////////////////////////////////////////////////////////////
/////////////   CONVERSION WIDGETS <-> PREFERENCES   /////////////
//////////////////////////////////////////////////////////////////

Settings SettingsDialog::preferencesFromWidgetValues() const
{
    Settings preferences = preferencesBak;
    preferences.setEdgeWidth( edgeWidth_->value() );
    return preferences;
}

void SettingsDialog::setWidgetValuesFromPreferences(const Settings & preferences)
{
    edgeWidth_->setValue( preferences.edgeWidth() );
}


//////////////////////////////////////////////////////////////////
//////////   ACTUALLY CHANGE APPLICATION PREFERENCES /////////////
//////////////////////////////////////////////////////////////////

void SettingsDialog::writeApplicationSettings()
{
    global()->settings() = preferencesFromWidgetValues();
    emit preferencesChanged();
}

void SettingsDialog::writeApplicationSettings(const Settings & settings)
{
    global()->settings() = settings;
    emit preferencesChanged();
}


//////////////////////////////////////////////////////////////////
//////////////            MAIN MECHANISM            //////////////
//////////////////////////////////////////////////////////////////

void SettingsDialog::go()
{
    preferencesBak = global()->settings();
    setWidgetValuesFromPreferences( global()->settings() );
    exec();
}

void SettingsDialog::dialogButtonClicked(QAbstractButton * button)
{
    QDialogButtonBox::StandardButton standardButton =
            dialogButtons_->standardButton(button);

    if(standardButton == QDialogButtonBox::Ok)
    {
        ok();
    }
    else if(standardButton == QDialogButtonBox::Apply)
    {
        apply();
    }
    else if(standardButton == QDialogButtonBox::Cancel)
    {
        cancel();
    }
}

void SettingsDialog::cancel()
{
    writeApplicationSettings(preferencesBak);
    reject();
}

void SettingsDialog::apply()
{
    preferencesBak = preferencesFromWidgetValues();
    writeApplicationSettings(preferencesBak);
}

void SettingsDialog::ok()
{
    apply();
    accept();
}
