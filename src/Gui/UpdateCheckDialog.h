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

class QCheckBox;

class UpdateCheckDialog: public QDialog
{
    Q_OBJECT

public:
    UpdateCheckDialog(QString newVersion, QWidget *parent = 0, Qt::WindowFlags f = 0);
    bool skipVersion();
    bool stopChecking();

private slots:
    void stopCheckingChanged_();

private:
    QCheckBox * skipVersionCheckBox_, * stopCheckingCheckBox_;
};

#endif // UPDATECHECKDIALOG_H
