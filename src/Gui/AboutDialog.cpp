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

#include "AboutDialog.h"

#include <QCheckBox>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTabWidget>
#include <QUrl>
#include <QVBoxLayout>
#include <QtNetwork>

PushLineEdit::PushLineEdit(QWidget * parent) :
    QWidget(parent)
{
    pushButton_ = new QPushButton();
    lineEdit_ = new QLineEdit();
    QHBoxLayout * layout = new QHBoxLayout();
    layout->addWidget(lineEdit_);
    layout->addWidget(pushButton_);
    setLayout(layout);
    setSizePolicy(lineEdit_->sizePolicy());
}

void PushLineEdit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        pushButton_->click();
    }
    else {
        QWidget::keyPressEvent(event);
    }
}

AboutDialog::AboutDialog(bool showAtStartup)
{
    QString aboutText = tr(
            "Version: %1<br><br>"
            " "
            "<a href=\"https://www.vpaint.org\" style=\"text-decoration: none;\">https://www.vpaint.org</a><br>"
            "<a href=\"https://www.vgc.io\" style=\"text-decoration: none;\">https://www.vgc.io</a><br><br>"
            " "
            "VPaint is an <b>experimental prototype</b> based on the Vector"
            " Graphics Complex (<b>VGC</b>), a technology developed by a"
            " collaboration of researchers at Inria and the University of"
            " British Columbia. It allows you to create resolution-independent"
            " illustrations and animations using innovative techniques.<br><br>"
            " "
            "<b>DISCLAIMER</b><br><br>"
            " "
            "VPaint is considered BETA, and always will: it"
            " lacks plenty of useful features commonly found in other editors, and you"
            " should expect glitches and crashes once in a while. It is distributed"
            " primarily for research purposes, and for curious artists interested in early"
            " testing of cutting-edge but unstable technology.<br><br>"
            " "
            "<b>VGC ACCOUNT</b><br><br>"
            " "
            "Because VPaint is just a prototype, we may or may not release new versions."
            " However, we are currently developing two new apps, called VGC Illustration and "
            " VGC Animation, based on what we learned from VPaint. These new apps will significantly "
            " improve performance compared to VPaint, and have more features. <b>If you would like to be notified if/when "
            " a new version of VPaint is released, or when VGC Illustration and VGC Animation are released</b>, you "
            " can sign up for a VGC account by entering your email address below, and following the "
            " instructions in the email which will be sent to you. Thanks for your interest, and have fun "
            " testing VPaint!").arg(qApp->applicationVersion());

    QString licenseText = tr(
            "Copyright (C) 2012-2019 The VPaint Developers.<br>"
            "See the COPYRIGHT file at <a href=\"https://github.com/dalboris/vpaint/blob/master/COPYRIGHT\">"
            "https://github.com/dalboris/vpaint/blob/master/COPYRIGHT</a><br><br>"
            " "
            "Licensed under the Apache License, Version 2.0 (the \"License\");"
            " you may not use this software except in compliance with the License."
            " You may obtain a copy of the License at:<br><br>"
            ""
            "     <a href=\"http://www.apache.org/licenses/LICENSE-2.0\">"
            "http://www.apache.org/licenses/LICENSE-2.0</a><br><br>"
            ""
            "Unless required by applicable law or agreed to in writing, software"
            " distributed under the License is distributed on an \"AS IS\" BASIS,"
            " WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied."
            " See the License for the specific language governing permissions and"
            " limitations under the License.");

    // Window title
    setWindowTitle(tr("About"));
    setMinimumSize(500, 500);

    // About widget
    QWidget * aboutWidget = new QWidget();

    QHBoxLayout * logoLayout = new QHBoxLayout();
    QPixmap logo(":/images/aboutlogo.png");
    QLabel * logoLabel = new QLabel();
    if (logo.width() > 0) {
        logoLabel->setPixmap(logo);
        logoLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        logoLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
        logoLabel->setMaximumSize(logo.width() / 2, logo.height() / 2);
        logoLabel->setScaledContents(true);
    }
    logoLayout->addStretch();
    logoLayout->addWidget(logoLabel);
    logoLayout->addStretch();

    QLabel * aboutLabel = new QLabel(aboutText);
    aboutLabel->setWordWrap(true);
    aboutLabel->setTextFormat(Qt::RichText);
    aboutLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    signup_ = new PushLineEdit();
    signup_->lineEdit()->setPlaceholderText(tr("Your email address"));
    signup_->pushButton()->setText(tr("Sign up!"));
    connect(signup_->pushButton(), SIGNAL(clicked(bool)), this, SLOT(processSignup_()));

    networkManager_ = new QNetworkAccessManager(this);

    QVBoxLayout * aboutWidgetLayout = new QVBoxLayout();
    aboutWidgetLayout->addLayout(logoLayout);
    aboutWidgetLayout->addSpacing(10);
    aboutWidgetLayout->addWidget(aboutLabel);
    aboutWidgetLayout->addSpacing(10);
    aboutWidgetLayout->addWidget(signup_);
    aboutWidgetLayout->addSpacing(10);
    aboutWidget->setLayout(aboutWidgetLayout);

    // License widget
    QWidget * licenseWidget = new QWidget();
    QLabel * licenseLabel = new QLabel(licenseText);
    licenseLabel->setWordWrap(true);
    licenseLabel->setTextFormat(Qt::RichText);
    licenseLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    licenseLabel->setOpenExternalLinks(true);

    QVBoxLayout * licenseWidgetLayout = new QVBoxLayout();
    licenseWidgetLayout->addWidget(licenseLabel);
    aboutWidgetLayout->addStretch();
    licenseWidget->setLayout(licenseWidgetLayout);

    // Tab widget
    QTabWidget * tabWidget =  new QTabWidget();
    tabWidget->addTab(aboutWidget, tr("About"));
    tabWidget->addTab(licenseWidget, tr("License"));

    // Show at startup checkbox
    showAtStartupCheckBox_ = new QCheckBox(tr("Show this message at startup"));
    showAtStartupCheckBox_->setChecked(showAtStartup);

    // Dialog button box
    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    // Main layout
    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget(tabWidget);
    layout->addWidget(showAtStartupCheckBox_);
    layout->addWidget(buttonBox);
    setLayout(layout);
}

void AboutDialog::processSignup_()
{
    // Set query
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("email", signup_->lineEdit()->text());
    QUrl url = QUrl("https://www.vgc.io/signup");
    QNetworkRequest networkRequest(url);
    networkRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                             "application/x-www-form-urlencoded; charset=utf-8");

    // Send query
    QString urlQueryString = urlQuery.toString(QUrl::FullyEncoded);
    urlQueryString.replace('+', "%2B");
    reply_ = networkManager_->post(networkRequest, urlQueryString.toUtf8());

    // Connection to process reply
    connect(reply_, SIGNAL(finished()), this, SLOT(processFinished_()));
}

void AboutDialog::processFinished_()
{
    QVariant status = reply_->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    QMessageBox msgBox(this);
    msgBox.setTextFormat(Qt::RichText);
    if (status.isValid() && status.toInt() == 200)
    {
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setWindowTitle(tr("Activate your VGC account"));
        msgBox.setText(tr(
            "Thank you!<br><br>"
            "Please check your inbox and"
            " <b>follow the remaining instructions</b> to activate your VGC account.<br><br>"
            "Contact us at <a href=\"mailto:support@vgc.io\">support@vgc.io</a>"
            " if you haven't received our email after a few minutes."));
        signup_->lineEdit()->clear();
    }
    else
    {
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle(tr("Error"));
        msgBox.setText(tr(
            "Oops... something went wrong. Please try again, or contact us"
            " at <a href=\"mailto:support@vgc.io\">support@vgc.io</a> if "
            " the problem persists."));
    }
    msgBox.exec();
    reply_->deleteLater();
}

bool AboutDialog::showAtStartup() const
{
    return showAtStartupCheckBox_->isChecked();
}
