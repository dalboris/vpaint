// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef UPDATECHECK_H
#define UPDATECHECK_H

#include <QObject>

#include "Version.h"

class QNetworkAccessManager;
class QNetworkReply;
class QWidget;
class UpdateCheckDialog;

class UpdateCheck: public QObject
{
    Q_OBJECT

public:
    UpdateCheck(QWidget * parent = 0);
    UpdateCheck(Version configVersion, QWidget * parent = 0);
    virtual ~UpdateCheck();

    Version versionChecked() const;
    Version latestVersion() const;
    void showWhenReady();
    void checkForUpdates();

private slots:
    void requestFinished_();
    void updateSettings_();

private:
    QNetworkAccessManager * networkManager_;
    QNetworkReply * reply_;

    QWidget * parent_;
    UpdateCheckDialog * dialog_;

    Version versionToCheck_, latestVersion_;
    bool isReady_ = false;
};

#endif // UPDATECHECK_H
