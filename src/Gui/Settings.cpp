// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "Settings.h"

#include <QSettings>

Settings::Settings()
{
}

void Settings::readFromDisk(QSettings & settings)
{
    edgeWidth_ = settings.value("tools-sketch-edgewidth", 10.0).toDouble();
    showAboutDialogAtStartup_ = settings.value("general-showaboutdialogatstartup", true).toBool();
    keepOldVersion_ = settings.value("general-keepoldversion", true).toBool();
    dontNotifyConversion_ = settings.value("general-dontnotifyconversion", false).toBool();
}

void Settings::writeToDisk(QSettings & settings)
{
    settings.setValue("tools-sketch-edgewidth", edgeWidth_);
    settings.setValue("general-showaboutdialogatstartup", showAboutDialogAtStartup_);
    settings.setValue("general-keepoldversion", keepOldVersion_);
    settings.setValue("general-dontnotifyconversion", dontNotifyConversion_);
}

// Edge width
double Settings::edgeWidth() const { return edgeWidth_; }
void Settings::setEdgeWidth(double value) { edgeWidth_ = value; }

// About dialog
bool Settings::showAboutDialogAtStartup() const { return showAboutDialogAtStartup_; }
void Settings::setShowAboutDialogAtStartup(bool value) { showAboutDialogAtStartup_ = value; }

// File Version Conversion
bool Settings::keepOldVersion() const { return keepOldVersion_; }
void Settings::setKeepOldVersion(bool value) { keepOldVersion_ = value; }

bool Settings::dontNotifyConversion() const { return dontNotifyConversion_; }
void Settings::setDontNotifyConversion(bool value) { dontNotifyConversion_ = value; }
