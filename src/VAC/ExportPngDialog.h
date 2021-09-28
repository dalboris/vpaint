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

#ifndef EXPORTPNGDIALOG_H
#define EXPORTPNGDIALOG_H

#include <QDialog>
#include "vpaint_global.h"

class QCheckBox;
class QDoubleSpinBox;
class QFormLayout;
class QSpinBox;
class Scene;

class Q_VPAINT_EXPORT ExportPngDialog: public QDialog
{
    Q_OBJECT

public:
    ExportPngDialog(Scene * scene);

    // Reimplements from QDialog
    void setVisible(bool visible);

    // Access linked scene
    Scene * scene() const;

    // Access png settings
    int pngWidth() const;
    int pngHeight() const;
    bool preserveAspectRatio() const;
    bool exportSequence() const;
    bool useViewSettings() const;

    // Motion blur
    bool motionBlur() const;
    int motionBlurNumSamples() const;

public slots:
    // Reimplements from QDialog
    void accept();
    void reject();

    // Backend <-> Frontend conversions
    void updateDialogFromScene();

private slots:
    void processPngWidthChanged_(int w);
    void processPngHeightChanged_(int h);
    void processPreserveAspectRatioChanged_(bool b);
    void processMotionBlurChanged_(bool b);

private:
    Scene * scene_;

    QSpinBox * pngWidthSpinBox_;
    QSpinBox * pngHeightSpinBox_;
    QCheckBox * preserveAspectRatioCheckBox_;
    QCheckBox * exportSequenceCheckBox_;
    QCheckBox * useViewSettings_;

    QCheckBox * motionBlurCheckBox_;
    QSpinBox * motionBlurNumSamplesSpinBox_;
    QFormLayout * motionBlurOptionsLayout_;

    bool ignoreWidthHeightChanged_;

    void enforcePngAspectRatio_();
    void setPngWidthForHeight_();
    void setPngHeightForWidth_();
};


#endif // EXPORTPNGDIALOG_H
