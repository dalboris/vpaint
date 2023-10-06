// Copyright (C) 2012-2023 The VPaint Developers.
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

#ifndef EXPORT_AS_DIALOG_H
#define EXPORT_AS_DIALOG_H

#include <QDialog>

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QFormLayout;
class QLineEdit;
class QRadioButton;
class QSpinBox;
class Scene;

class ExportAsDialog: public QDialog
{
    Q_OBJECT

public:
    ExportAsDialog(Scene * scene);

    // Reimplements from QDialog
    void setVisible(bool visible);

    // Access linked scene
    Scene * scene() const;

    // Access png settings
    int outWidth() const;
    int outHeight() const;
    bool preserveAspectRatio() const;
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

protected:
    void showEvent(QShowEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    // Output files
    void processFilenameLineEditEditingFinished_();
    void processFilenameBrowseButtonClicked_();

    void processOutWidthChanged_(int w);
    void processOutHeightChanged_(int h);
    void processPreserveAspectRatioChanged_(bool b);
    void processMotionBlurChanged_(bool b);

private:
    Scene * scene_;

    QComboBox * fileFormatComboBox_;

    QButtonGroup * frameRangeGroup_;
    QRadioButton * singleImage_;
    QRadioButton * imageSequenceAll_;
    //QRadioButton * imageSequenceCustom_;
    QLineEdit * filenameLineEdit_;
    QPushButton * filenameBrowseButton_;

    QSpinBox * outWidthSpinBox_;
    QSpinBox * outHeightSpinBox_;
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


#endif // ExportAsDIALOG_H
