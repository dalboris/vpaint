// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "UpdateCheck.h"

#include <QUrl>
#include <QtNetwork>
#include <QDebug>

#include "Application.h"
#include "Global.h"
#include "UpdateCheckDialog.h"

UpdateCheck::UpdateCheck(QWidget * parent) :
    reply_(0)
{
    // Initialize variables
    versionToCheck_ = global()->settings().checkVersion();
    networkManager_ = new QNetworkAccessManager();
    parent_ = parent;

    checkForUpdates();
}

UpdateCheck::UpdateCheck(Version configVersion, QWidget * parent) :
    reply_(0)
{
    // Initialize variables
    versionToCheck_ = configVersion;
    networkManager_ = new QNetworkAccessManager();
    parent_ = parent;

    checkForUpdates();
}

UpdateCheck::~UpdateCheck()
{
    delete networkManager_;
}

Version UpdateCheck::versionChecked() const
{
    return versionToCheck_;
}

Version UpdateCheck::latestVersion() const
{
    return latestVersion_;
}

void UpdateCheck::checkForUpdates()
{
    // Return if a request is already in progress
    if(reply_) return;

    // Return if the user has asked not to check for updates
    if(versionToCheck_ == Version()) return;

    // Set query
    QUrlQuery urlQuery;
    QUrl url = QUrl("http://vpaint.org/latestversion.php");
    QNetworkRequest networkRequest(url);
    networkRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                             "application/x-www-form-urlencoded; charset=utf-8");

    // Send query
    QString urlQueryString = urlQuery.toString(QUrl::FullyEncoded);
    urlQueryString.replace('+', "%2B");
    reply_ = networkManager_->post(networkRequest, urlQueryString.toUtf8());

    // Connection to process reply
    connect(reply_, SIGNAL(finished()), this, SLOT(requestFinished_()));
}

void UpdateCheck::requestFinished_()
{
    // Return if not ready to display dialog
    if(!isReady_) return;

    // Check for errors
    if(reply_->error() != QNetworkReply::NoError)
    {
        qDebug() << "Warning: could not check for updates. " << reply_->errorString();
        return;
    }

    // Read and parse response
    latestVersion_ = Version(QString(reply_->readAll()));

    // Compare versions
    if(versionToCheck_ < latestVersion_)
    {
        // Create dialog with latest version
        dialog_ = new UpdateCheckDialog(latestVersion_.toString(), parent_, Qt::Dialog);
        dialog_->setAttribute(Qt::WA_DeleteOnClose);
        connect(dialog_, SIGNAL(accepted()), this, SLOT(updateSettings_()));

        // Display dialog
        dialog_->exec();
    }

    reply_->deleteLater();

    isReady_ = false;
}

void UpdateCheck::showWhenReady()
{
    if(reply_ && reply_->isFinished())
    {
        isReady_ = true;
        requestFinished_();
    }
    else {
        isReady_ = true;
    }
}

void UpdateCheck::updateSettings_() {
    if(dialog_->stopChecking())
    {
        global()->settings().setCheckVersion(Version());
    }
    else if(dialog_->skipVersion()) {
        global()->settings().setCheckVersion(latestVersion_);
    }
}
