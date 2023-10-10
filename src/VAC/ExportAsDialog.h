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

/// \class ExportFileTypeInfo
/// \brief Specifies broad categories of file types.
///
enum class ExportFileTypeCategory {
    RasterImage,
    VectorImage
    // RasterVideo
    // VectorVideo
};

/// \class ExportFileTypeInfo
/// \brief Specifies meta-information about a given file type.
///
///
class ExportFileTypeInfo
{
public:
    /// Creates an `ExportFileTypeInfo`, with the given `extension` (without
    /// the leading dot) and the given `name`.
    ///
    ExportFileTypeInfo(
        const std::string& extension,
        const std::string& name,
        ExportFileTypeCategory category)

        : extension_(extension)
        , name_(name)
        , category_(category) {
    }

    /// Returns the extension of this file type (without the leading dot).
    ///
    /// Example: `svg`.
    ///
    /// Note that two different `ExportFileTypeInfo` may have the same
    /// extension. For example, exporting as an SVG image sequence or as an SVG
    /// animation (SMIL) are two different export options with different
    /// `category()`: the first is `VectorImage`, while the second is
    /// `VectorVideo`.
    ///
    const std::string& extension() const {
        return extension_;
    }

    /// Returns the name of this file type.
    ///
    /// Example: `SVG Image`.
    ///
    const std::string& name() const {
        return name_;
    }

    /// Returns the category of this file type.
    ///
    ExportFileTypeCategory category() const {
        return category_;
    }

private:
    std::string extension_;
    std::string name_;
    ExportFileTypeCategory category_;
};

/// Returns a list of registered file types.
///
const std::vector<ExportFileTypeInfo>& exportFileTypes();

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

    QComboBox* fileFormatComboBox_;

    QButtonGroup* frameRangeGroup_;
    QRadioButton* singleImage_;
    QRadioButton* imageSequenceAll_;
    //QRadioButton * imageSequenceCustom_;
    QLineEdit* filenameLineEdit_;
    QPushButton* filenameBrowseButton_;
    bool hasExplicitExportFilename_ = false;

    QSpinBox* outWidthSpinBox_;
    QSpinBox* outHeightSpinBox_;
    QCheckBox* preserveAspectRatioCheckBox_;
    QCheckBox* exportSequenceCheckBox_;
    QCheckBox* useViewSettings_;

    QCheckBox* motionBlurCheckBox_;
    QSpinBox* motionBlurNumSamplesSpinBox_;
    QFormLayout* motionBlurOptionsLayout_;

    bool ignoreWidthHeightChanged_;

    void enforcePngAspectRatio_();
    void setPngWidthForHeight_();
    void setPngHeightForWidth_();
};

#endif // EXPORT_AS_DIALOG_H
