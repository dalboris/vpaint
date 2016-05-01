// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef FILEVERSIONCONVERTERDIALOG_H
#define FILEVERSIONCONVERTERDIALOG_H

#include <QDialog>

class QCheckBox;

class FileVersionConverterDialog: public QDialog
{
    Q_OBJECT

public:
    FileVersionConverterDialog(QWidget * parent,
                               const QString & fileName,
                               const QString & backupName);

private slots:
    void keepOldVersionToggled(bool checked);
    void dontNotifyConversionToggled(bool checked);
};

#endif // FILEVERSIONCONVERTERDIALOG_H
