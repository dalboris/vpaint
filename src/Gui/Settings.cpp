// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "Application.h"
#include "Settings.h"

#include <QSettings>

Settings::Settings()
{
}

void Settings::readFromDisk(QSettings & settings)
{
    edgeWidth_ = settings.value("tools-sketch-edgewidth", 10.0).toDouble();
    showAboutDialogAtStartup_ = settings.value("general-showaboutdialogatstartup", true).toBool();
    checkVersion_ = Version(settings.value("general-checkversion", qApp->applicationVersion()).toString());
}

void Settings::writeToDisk(QSettings & settings)
{
    settings.setValue("tools-sketch-edgewidth", edgeWidth_);
    settings.setValue("general-showaboutdialogatstartup", showAboutDialogAtStartup_);
    settings.setValue("general-checkversion", checkVersion_.toString());
}

// Edge width
double Settings::edgeWidth() const { return edgeWidth_; }
void Settings::setEdgeWidth(double value) { edgeWidth_ = value; }

// About dialog
bool Settings::showAboutDialogAtStartup() const { return showAboutDialogAtStartup_; }
void Settings::setShowAboutDialogAtStartup(bool value) { showAboutDialogAtStartup_ = value; }

// Check version
Version Settings::checkVersion() const { return checkVersion_; }
void Settings::setCheckVersion(Version value) { checkVersion_ = value; }
