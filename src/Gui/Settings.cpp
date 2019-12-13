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
    keepOldVersion_ = settings.value("general-keepoldversion", true).toBool();
    dontNotifyConversion_ = settings.value("general-dontnotifyconversion", false).toBool();
    checkVersion_ = Version(settings.value("general-checkversion", qApp->applicationVersion()).toString());
    svgImportVertexMode_ = toSvgImportVertexMode(settings.value("svgimport-vertexmode", toString(defaultSvgImportVertexMode)).toString());
}

void Settings::writeToDisk(QSettings & settings)
{
    settings.setValue("tools-sketch-edgewidth", edgeWidth_);
    settings.setValue("general-showaboutdialogatstartup", showAboutDialogAtStartup_);
    settings.setValue("general-keepoldversion", keepOldVersion_);
    settings.setValue("general-dontnotifyconversion", dontNotifyConversion_);
    settings.setValue("general-checkversion", checkVersion_.toString());
    settings.setValue("svgimport-vertexmode", toString(svgImportVertexMode_));
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

// Check version
Version Settings::checkVersion() const { return checkVersion_; }
void Settings::setCheckVersion(Version value) { checkVersion_ = value; }

// Import preferences
SvgImportVertexMode Settings::svgImportVertexMode() const { return svgImportVertexMode_; }
void Settings::setSvgImportVertexMode(SvgImportVertexMode value) { svgImportVertexMode_ = value; }
