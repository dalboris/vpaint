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

#include "ExportSettings.h"

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QFormLayout;
class QGroupBox;
class QLineEdit;
class QRadioButton;
class QSpinBox;
class Scene;

class ExportAsDialog : public QDialog {
    Q_OBJECT

public:
    ExportAsDialog(Scene* scene);

    // Reimplements from QDialog
    void setVisible(bool visible) override;

    // Access linked scene
    Scene* scene() const;

    /// Returns the number of available file types.
    ///
    int numFileTypes() const;

    /// Returns information on the i-th available file type,
    /// or nullptr if `i` is out of range.
    ///
    const ExportFileTypeInfo* fileTypeInfo(int i) const;

    /// Information on the currently selected file type,
    /// or nullptr if no selected file type.
    ///
    const ExportFileTypeInfo* fileTypeInfo() const;

    /// Returns the export filename.
    ///
    QString filePath() const;

    /// Returns whether to export a single image or an image sequence.
    ///
    FrameRangeType frameRangeType() const;

    /// Returns the current raster settings as displayed in this dialog.
    ///
    RasterExportSettings rasterSettings() const;

    /// Returns the current vector settings as displayed in this dialog.
    ///
    VectorExportSettings vectorSettings() const;

    // Individual settings
    int outWidth() const;
    int outHeight() const;
    bool preserveAspectRatio() const;
    bool useViewSettings() const;
    bool motionBlur() const;
    int motionBlurNumSamples() const;

public slots:
    // Reimplements from QDialog
    void accept();
    void reject();

    // Backend <-> Frontend conversions
    void updateDialogFromScene();
    void updateFilenameFromDocumentName();

protected:
    void showEvent(QShowEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    // Output files
    void onFileTypeChanged_();
    void onFilenameLineEditEditingFinished_();
    void onFilenameBrowseButtonClicked_();
    void onFrameRangeTypeChanged_();
    void updateFilename_(bool isManualEdit = false);

    void onOutWidthChanged_(int w);
    void onOutHeightChanged_(int h);
    void onPreserveAspectRatioChanged_(bool b);
    void onMotionBlurChanged_(bool b);


private:
    Scene* scene_;

    // File format and filename selection
    QComboBox* fileFormatComboBox_;
    QButtonGroup* frameRangeGroup_;
    QRadioButton* singleImage_;
    QRadioButton* imageSequenceAll_;
    QLineEdit* filenameLineEdit_;
    QPushButton* filenameBrowseButton_;
    bool hasExplicitExportFilename_ = false;

    // Raster settings
    QGroupBox* rasterSettingsBox_;
    QSpinBox* outWidthSpinBox_;
    QSpinBox* outHeightSpinBox_;
    QCheckBox* preserveAspectRatioCheckBox_;
    QCheckBox* useViewSettings_;
    QCheckBox* motionBlurCheckBox_;
    QSpinBox* motionBlurNumSamplesSpinBox_;
    QFormLayout* motionBlurOptionsLayout_;

    bool ignoreWidthHeightChanged_;

    void enforcePngAspectRatio_();
    void setPngWidthForHeight_();
    void setPngHeightForWidth_();

    // Vector settings
    QGroupBox* vectorSettingsBox_;
    QCheckBox* backgroundAsRect_;
    QCheckBox* fillVariableWidthStrokes_;
};

#endif // EXPORT_AS_DIALOG_H
