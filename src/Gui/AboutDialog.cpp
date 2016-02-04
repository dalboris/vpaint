// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "AboutDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QLabel>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
#include <QtNetwork>

AboutDialog::AboutDialog(bool showAtStartup)
{
    QString websiteText = tr("<a href=\"http://www.vpaint.org\">http://www.vpaint.org</a>");

    QString aboutText = tr(
            "VPaint is an experimental vector graphics editor based on the Vector Animation"
            " Complex (VAC), a technology developed by a collaboration of researchers at"
            " Inria and the University of British Columbia, featured at SIGGRAPH 2015. It"
            " allows you to create resolution-independent illustrations and animations using"
            " innovative techniques.<br><br>"
            " "
            "<b>DISCLAIMER:</b> VPaint 1.5, or any upcoming 1.x release, is considered BETA. It"
            " lacks plenty of useful features commonly found in other editors, and you"
            " should expect glitches and crashes once in a while. <b>It is distributed"
            " primarily for research purposes, and for curious artists interested in early"
            " testing of cutting-edge but unstable technology.</b><br><br>"
            " "
            "New VPaint 1.x versions (1.6, 1.7, etc.) are expected every two months, with"
            " new features and increased stability. VPaint 2.0, full-featured and stable, is"
            " expected for 2017. <b>If you want to be notified when a new version is released,"
            " just enter your email address below.</b>");

    QString licenseText = tr(
            "Copyright (C) 2012-2015 Boris Dalstein.<br><br>"
            " "
            "Permission is hereby granted, free of charge, to any person obtaining a copy"
            " of this software and associated documentation files (the \"Software\"), to deal"
            " in the Software without restriction, including without limitation the rights"
            " to use, copy, modify, merge, publish, distribute, sublicense, and/or sell"
            " copies of the Software, and to permit persons to whom the Software is"
            " furnished to do so, subject to the following conditions:<br><br>"
            " "
            "The above copyright notice and this permission notice shall be included in"
            " all copies or substantial portions of the Software.<br><br>"
            " "
            "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR"
            " IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,"
            " FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE"
            " AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER"
            " LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,"
            " OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN"
            " THE SOFTWARE.");

    // Window title
    setWindowTitle(tr("About"));
    setMinimumSize(500, 500);

    // About widget
    QWidget * aboutWidget = new QWidget();

    QLabel * logoLabel = new QLabel();
    logoLabel->setPixmap(QPixmap(":/images/logo_1-5.png"));
    logoLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    QLabel * websiteLabel = new QLabel(websiteText);
    websiteLabel->setTextFormat(Qt::RichText);
    websiteLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    websiteLabel->setOpenExternalLinks(true);
    QFont websiteFont = websiteLabel->font();
    websiteFont.setPointSize(15);
    websiteLabel->setFont(websiteFont);

    QLabel * aboutLabel = new QLabel(aboutText);
    aboutLabel->setWordWrap(true);
    aboutLabel->setTextFormat(Qt::RichText);
    aboutLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    subscribeLineEdit_ = new QLineEdit();
    subscribeLineEdit_->setPlaceholderText(tr("Your email address"));
    networkManager_ = new QNetworkAccessManager(this);

    QPushButton * subscribeButton = new QPushButton(tr("Subscribe"));
    connect(subscribeButton, SIGNAL(clicked(bool)), this, SLOT(processSubscribe_()));

    QHBoxLayout * subscribeLayout = new QHBoxLayout();
    subscribeLayout->addWidget(subscribeLineEdit_);
    subscribeLayout->addWidget(subscribeButton);

    QVBoxLayout * aboutWidgetLayout = new QVBoxLayout();
    aboutWidgetLayout->addWidget(logoLabel);
    aboutWidgetLayout->addSpacing(10);
    aboutWidgetLayout->addWidget(websiteLabel);
    aboutWidgetLayout->addSpacing(10);
    aboutWidgetLayout->addWidget(aboutLabel);
    aboutWidgetLayout->addSpacing(10);
    aboutWidgetLayout->addLayout(subscribeLayout);
    aboutWidgetLayout->addStretch();
    aboutWidget->setLayout(aboutWidgetLayout);

    // License widget
    QWidget * licenseWidget = new QWidget();
    QLabel * licenseLabel = new QLabel(licenseText);
    licenseLabel->setWordWrap(true);
    licenseLabel->setTextFormat(Qt::RichText);
    licenseLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);

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

void AboutDialog::processSubscribe_()
{
    // Set query
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("emailaddress", subscribeLineEdit_->text());
    QUrl url = QUrl("http://www.vpaint.org/subscribeext.php");
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
    QString okMessage = tr(
                "Thank you for your interest in VPaint! You should receive an email shortly"
                " which confirms your subscription.");

    QString invalidemailMessage = tr(
                "Sorry, it seems that your email address is invalid. Please try again,"
                " and if you are still having issues, contact me at dalboris@cs.ubc.ca.");

    QString errorMessage = tr(
                "Oops... it seems something went wrong, I suggest that you try again. If"
                " you are still having issues,"
                " then I apologize for the inconvenience. Please contact me (Boris Dalstein) at"
                " dalboris@cs.ubc.ca and I'll be happy to help you out.");


    QString replyString = reply_->readAll();
    if(replyString == QString("ok"))
    {
        QMessageBox::information(this, tr("Subscription successful"), okMessage, QMessageBox::Ok);
        subscribeLineEdit_->clear();
    }
    else if(replyString == QString("invalidemail"))
    {
        QMessageBox::warning(this, tr("Subscription failed"), invalidemailMessage, QMessageBox::Ok);
    }
    else if(replyString == QString("invalidemail"))
    {
        QMessageBox::warning(this, tr("Subscription failed"), errorMessage, QMessageBox::Ok);
    }
    else
    {
        QMessageBox::warning(this, tr("Subscription failed"), errorMessage, QMessageBox::Ok);
    }

    reply_->deleteLater();
}

bool AboutDialog::showAtStartup() const
{
    return showAtStartupCheckBox_->isChecked();
}
