// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef UPDATECHECKDIALOG_H
#define UPDATECHECKDIALOG_H

#include <QDialog>
#include <QLabel>

#include "Version.h"

class QLineEdit;
class QCheckBox;
class QNetworkAccessManager;
class QNetworkReply;

class UpdateCheckDialog: public QDialog
{
    Q_OBJECT

public:
    UpdateCheckDialog(Version configVersion);

    Version versionChecked() const;
    Version latestVersion() const;
    void checkForUpdates();
    void showWhenReady();

private slots:
    void requestFinished_();
    void stopCheckingChanged_(int state);
    void updateSettings(int result);

private:
    QLabel * mainDesc_;
    QCheckBox * skipVersionCheckBox_, * stopCheckingCheckBox_;

    QNetworkAccessManager * networkManager_;
    QNetworkReply * reply_;

    Version versionToCheck_, latestVersion_;
    bool isReady_;
};

#endif // UPDATECHECKDIALOG_H
