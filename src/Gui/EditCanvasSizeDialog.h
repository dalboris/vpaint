// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef EDITCANVASSIZEDIALOG_H
#define EDITCANVASSIZEDIALOG_H

#include <QDialog>

class QDoubleSpinBox;
class Scene;

class EditCanvasSizeDialog: public QDialog
{
    Q_OBJECT

public:
    EditCanvasSizeDialog(Scene * scene);

    Scene * scene() const;

    // Reimplements from QDialog to set oldValues
    void setVisible(bool visible);

    void backupCurentValues();

public slots:
    void updateDialogFromScene();
    void updateSceneFromDialog();

    void accept();
    void reject();

private:
    Scene * scene_;

    QDoubleSpinBox * topSpinBox_;
    QDoubleSpinBox * leftSpinBox_;
    QDoubleSpinBox * widthSpinBox_;
    QDoubleSpinBox * heightSpinBox_;

    double oldTop_;
    double oldLeft_;
    double oldWidth_;
    double oldHeight_;

    bool ignoreSceneChanged_;
};

#endif // EDITCANVASSIZEDIALOG_H
