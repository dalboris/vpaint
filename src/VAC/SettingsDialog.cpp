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
