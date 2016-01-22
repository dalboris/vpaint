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

#include "Application.h"
#include "Global.h"

UpdateCheckDialog::UpdateCheckDialog(QString newVersion, QWidget *parent, Qt::WindowFlags f)
{
    setParent(parent, f);
    setWindowTitle(tr("Updates Available"));
    setMinimumSize(500, 200);

    QString updateText = tr(
            "An update is available for VPaint. The current version is %1"
            " and the newest version of VPaint is %2.<br><br>"
            " "
            "The latest version for your system can be downloaded"
            " <a href=\"$DOWNLOAD_LINK\">here</a>.").arg(qApp->applicationVersion(), newVersion);

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
    QLabel * mainDesc = new QLabel(updateText);
    mainDesc->setWordWrap(true);
    mainDesc->setTextFormat(Qt::RichText);
    mainDesc->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    mainDesc->setOpenExternalLinks(true);

    // Skip version checkbox
    skipVersionCheckBox_ = new QCheckBox(QString("Don't remind me about this version again"));
    skipVersionCheckBox_->setChecked(false);

    // Stop checking checkbox
    stopCheckingCheckBox_ = new QCheckBox(QString("Stop checking for updates"));
    stopCheckingCheckBox_->setChecked(false);
    connect(stopCheckingCheckBox_, SIGNAL(stateChanged(int)), this, SLOT(stopCheckingChanged_()));

    // Dialog button
    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    // Main layout
    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget(mainDesc);
    layout->addWidget(skipVersionCheckBox_);
    layout->addWidget(stopCheckingCheckBox_);
    layout->addWidget(buttonBox);
    setLayout(layout);
}

bool UpdateCheckDialog::skipVersion()
{
    return skipVersionCheckBox_->checkState() == Qt::Checked;
}

bool UpdateCheckDialog::stopChecking()
{
    return stopCheckingCheckBox_->checkState() == Qt::Checked;
}

void UpdateCheckDialog::stopCheckingChanged_()
{
    if(stopChecking())
    {
        skipVersionCheckBox_->setDisabled(true);
    }
    else
    {
        skipVersionCheckBox_->setDisabled(false);
    }
}
