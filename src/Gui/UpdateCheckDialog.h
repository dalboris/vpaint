// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

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
