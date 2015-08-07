// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

class QLineEdit;
class QCheckBox;
class QNetworkAccessManager;
class QNetworkReply;

class AboutDialog: public QDialog
{
    Q_OBJECT

public:
    AboutDialog(bool showAtStartup);

    bool showAtStartup() const;

private slots:
    void processSubscribe_();
    void processFinished_();

private:
    QNetworkAccessManager * networkManager_;
    QNetworkReply * reply_;

    QLineEdit * subscribeLineEdit_;
    QCheckBox * showAtStartupCheckBox_;
};

#endif // ABOUTDIALOG_H
