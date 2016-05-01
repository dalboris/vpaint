// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef SETTINGS_H
#define SETTINGS_H

#include "UpdateCheckDialog.h"
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

private:
    double edgeWidth_;
    bool showAboutDialogAtStartup_;
    bool keepOldVersion_;
    bool dontNotifyConversion_;
    Version checkVersion_;
};

#endif
