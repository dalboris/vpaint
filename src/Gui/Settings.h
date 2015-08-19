// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef SETTINGS_H
#define SETTINGS_H

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

private:
    double edgeWidth_;
    bool showAboutDialogAtStartup_;
};

#endif
