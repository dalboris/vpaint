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
