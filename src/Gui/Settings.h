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

#ifndef SETTINGS_H
#define SETTINGS_H

#include "SvgImportParams.h"
#include "Version.h"

class SettingsDialog;
class QSettings;

class Settings
{
public:
    Settings();

    // Read and write from/to hard drive
    void readFromDisk(QSettings & settings);
    void writeToDisk(QSettings & settings);

    // Edge width
    double edgeWidth() const;
    void setEdgeWidth(double value);

    // About dialog
    bool showAboutDialogAtStartup() const;
    void setShowAboutDialogAtStartup(bool value);

    // File version conversion
    bool keepOldVersion() const;
    void setKeepOldVersion(bool value);

    bool dontNotifyConversion() const;
    void setDontNotifyConversion(bool value);

    // Check version
    Version checkVersion() const;
    void setCheckVersion(Version value);

    // Import Preferences
    SvgImportVertexMode svgImportVertexMode() const;
    void setSvgImportVertexMode(SvgImportVertexMode value);

private:
    double edgeWidth_;
    bool showAboutDialogAtStartup_;
    bool keepOldVersion_;
    bool dontNotifyConversion_;
    Version checkVersion_;
    SvgImportVertexMode svgImportVertexMode_;
};

#endif
