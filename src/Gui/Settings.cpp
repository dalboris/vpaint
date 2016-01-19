// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "Settings.h"

#include <QSettings>

Settings::Settings()
{
}

void Settings::readFromDisk(QSettings & settings)
{
    edgeWidth_ = settings.value("tools-sketch-edgewidth", 10.0).toDouble();
    showAboutDialogAtStartup_ = settings.value("general-showaboutdialogatstartup", true).toBool();
}

void Settings::writeToDisk(QSettings & settings)
{
    settings.setValue("tools-sketch-edgewidth", edgeWidth_);
    settings.setValue("general-showaboutdialogatstartup", showAboutDialogAtStartup_);
}

// Edge width
double Settings::edgeWidth() const { return edgeWidth_; }
void Settings::setEdgeWidth(double value) { edgeWidth_ = value; }

// About dialog
bool Settings::showAboutDialogAtStartup() const { return showAboutDialogAtStartup_; }
void Settings::setShowAboutDialogAtStartup(bool value) { showAboutDialogAtStartup_ = value; }
