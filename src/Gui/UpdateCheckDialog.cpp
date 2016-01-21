// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "UpdateCheckDialog.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
#include <QtNetwork>
#include <QDebug>

#include "Application.h"
#include "Global.h"

UpdateCheckDialog::UpdateCheckDialog(Version configVersion)
{
    versionToCheck_ = configVersion;
    networkManager_ = new QNetworkAccessManager();

    setWindowTitle(tr("Updates Available"));
    setMinimumSize(500, 200);

    QString updateText = tr(
            "An update is available for VPaint. The current version is %1"
            " and the newest version of VPaint is $NEW_VERSION.<br><br>"
            " "
            "The latest version for your system can be downloaded"
            " <a href=\"$DOWNLOAD_LINK\">here</a>.").arg(qApp->applicationVersion());

    // Insert the os specific download link into updateText
#ifdef Q_OS_WIN32
    updateText.replace(QString("$DOWNLOAD_LINK"), QString("https://github.com/dalboris/vpaint/releases/download/v$NEW_VERSION/VPaint$NEW_VERSION.Setup.msi"));
#endif
#ifdef Q_OS_MAC
    updateText.replace(QString("$DOWNLOAD_LINK"), QString("https://github.com/dalboris/vpaint/releases/download/v$NEW_VERSION/vpaint.dmg"));
#else
    updateText.replace(QString("$DOWNLOAD_LINK"), QString("https://github.com/dalboris/vpaint/releases/tag/v$NEW_VERSION"));
#endif

    // Main description
    mainDesc_ = new QLabel(updateText);
    mainDesc_->setWordWrap(true);
    mainDesc_->setTextFormat(Qt::RichText);
    mainDesc_->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    mainDesc_->setOpenExternalLinks(true);

    // Skip version checkbox
    skipVersionCheckBox_ = new QCheckBox(QString("Don't remind me about this version again"));
    skipVersionCheckBox_->setChecked(false);

    // Stop checking checkbox
    stopCheckingCheckBox_ = new QCheckBox(QString("Stop checking for updates"));
    stopCheckingCheckBox_->setChecked(false);
    connect(stopCheckingCheckBox_, SIGNAL(stateChanged(int)), this, SLOT(stopCheckingChanged_(int)));

    // Dialog button
    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    connect(this, SIGNAL(finished(int)), this, SLOT(updateSettings(int)));

    // Main layout
    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget(mainDesc_);
    layout->addWidget(skipVersionCheckBox_);
    layout->addWidget(stopCheckingCheckBox_);
    layout->addWidget(buttonBox);
    setLayout(layout);
}

Version UpdateCheckDialog::versionChecked() const
{
    return versionToCheck_;
}

Version UpdateCheckDialog::latestVersion() const
{
    return latestVersion_;
}

void UpdateCheckDialog::checkForUpdates()
{
    // Set query
    QUrl url = QUrl("http://vpaint.org/latestversion");
    QNetworkRequest networkRequest(url);

    // Send query
    reply_ = networkManager_->get(networkRequest);

    // Connection to process reply
    connect(reply_, SIGNAL(finished()), this, SLOT(requestFinished_()));
}

void UpdateCheckDialog::requestFinished_()
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
        // Set version for description
        mainDesc_->setText(mainDesc_->text().replace("$NEW_VERSION", latestVersion_.toString()));

        // Display dialog
        exec();
    }
}

void UpdateCheckDialog::showWhenReady()
{
    isReady_ = true;
    if(reply_ && reply_->isFinished()) {
        requestFinished_();
    }
}

void UpdateCheckDialog::stopCheckingChanged_(int state)
{
    if(state == Qt::Checked)
    {
        skipVersionCheckBox_->setDisabled(true);
    }
    else
    {
        skipVersionCheckBox_->setDisabled(false);
    }
}

void UpdateCheckDialog::updateSettings(int result) {
    if(result == QDialog::Accepted)
    {
        if(stopCheckingCheckBox_->checkState() == Qt::Checked)
        {
            global()->settings().setCheckVersion(Version());
        }
        else if(skipVersionCheckBox_->checkState() == Qt::Checked) {
            global()->settings().setCheckVersion(latestVersion_);
        }
    }
}
