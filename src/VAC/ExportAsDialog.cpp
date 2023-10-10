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

#include "ExportAsDialog.h"

#include "Global.h"
#include "Scene.h"

#include <algorithm> // max
#include <cmath>

#include <QButtonGroup>
#include <QComboBox>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QTextEdit>
#include <QVBoxLayout>

namespace {

std::vector<ExportFileTypeInfo> createFileTypes() {
    std::vector<ExportFileTypeInfo> types;
    using C = ExportFileTypeCategory;
    types.emplace_back("svg", "SVG Image", C::VectorImage);
    types.emplace_back("png", "PNG Image", C::RasterImage);
    return types;
}

} // namespace

const std::vector<ExportFileTypeInfo>& exportFileTypes() {
    static std::vector<ExportFileTypeInfo> types = createFileTypes();
    return types;
}

namespace {

QRadioButton * createRadioButton(QString label, QButtonGroup * group, QFormLayout * layout)
{
    QRadioButton * res = new QRadioButton(label);
    res->setFocusPolicy(Qt::NoFocus);
    group->addButton(res);
    layout->addRow(res);
    return res;
}

QCheckBox * createCheckBox(QString label, bool isChecked, QLayout * layout)
{
    QCheckBox * res = new QCheckBox(label);
    res->setFocusPolicy(Qt::NoFocus);
    res->setChecked(isChecked);
    layout->addWidget(res);
    return res;
}

} // namespace

ExportAsDialog::ExportAsDialog(Scene * scene) :
    scene_(scene),
    ignoreWidthHeightChanged_(false)
{
    // General dialog properties
    setWindowTitle(tr("Export As"));
    setMinimumWidth(350);

    // Give the focus back to the dialog when clicking on it.
    // See keyPressEvent() override for why this is important.
    setFocusPolicy(Qt::ClickFocus);

    QGroupBox * outputFilesGroupBox = new QGroupBox(tr("Output File(s)"));
    QFormLayout * outputFilesLayout = new QFormLayout();
    outputFilesGroupBox->setLayout(outputFilesLayout);

    // File format
    fileFormatComboBox_ = new QComboBox();
    for (const ExportFileTypeInfo& info : exportFileTypes()) {
        QString itemName;
        itemName.append(info.name().data());
        itemName.append(" (*.");
        itemName.append(info.extension().data());
        itemName.append(")");
        fileFormatComboBox_->addItem(itemName);
    }
    outputFilesLayout->addRow(tr("File Format:"), fileFormatComboBox_);
    connect(fileFormatComboBox_, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onFileTypeChanged_()));

    // Filename(s)
    filenameLineEdit_ = new QLineEdit();
    QString filenameTip = tr(
        "Specify output file path(s), relative to current VPaint file.\n"
        "The character `*`, if any, will be replaced by the frame number.");
    QString browseTip = tr(
        "Browse to select a file path where to export.");
    filenameLineEdit_->setToolTip(filenameTip);
    filenameLineEdit_->setStatusTip(filenameTip.replace('\n', ' '));
    filenameBrowseButton_ = new QPushButton("...");
    filenameBrowseButton_->setToolTip(browseTip);
    filenameBrowseButton_->setStatusTip(browseTip.replace('\n', ' '));
    filenameBrowseButton_->setMaximumWidth(30);
    QHBoxLayout * filenameLayout = new QHBoxLayout();
#ifdef Q_OS_MAC
    filenameLayout->setSpacing(10);
#else
    filenameLayout->setSpacing(0);
#endif
    filenameLayout->addWidget(filenameLineEdit_);
    filenameLayout->addWidget(filenameBrowseButton_);
    outputFilesLayout->addRow(tr("File Name(s):"), filenameLayout);
    connect(filenameLineEdit_, SIGNAL(editingFinished()),
            this, SLOT(onFilenameLineEditEditingFinished_()));
    connect(filenameBrowseButton_, SIGNAL(clicked(bool)),
            this, SLOT(onFilenameBrowseButtonClicked_()));

    // Single image vs. Image sequence
    frameRangeGroup_ = new QButtonGroup();
    singleImage_ = createRadioButton(
        tr("Single image (current frame)"), frameRangeGroup_, outputFilesLayout);
    imageSequenceAll_ = createRadioButton(
        tr("Image sequence (all frames)"), frameRangeGroup_, outputFilesLayout);
    singleImage_->setChecked(true);
    connect(frameRangeGroup_, SIGNAL(buttonClicked(int)),
            this, SLOT(onFrameRangeTypeChanged_()));

    // Raster settings
    rasterSettingsBox_ = new QGroupBox(tr("Options"));
    QVBoxLayout * rasterSettingsLayout = new QVBoxLayout();
    rasterSettingsBox_->setLayout(rasterSettingsLayout);

    // Image output size
    QFormLayout * outWidthHeightLayout = new QFormLayout();
    rasterSettingsLayout->addLayout(outWidthHeightLayout);
    outWidthSpinBox_ = new QSpinBox();
    outWidthSpinBox_->setRange(1,100000);
    outWidthSpinBox_->setValue(1280);
    outWidthSpinBox_->setMaximumWidth(60);
    outWidthHeightLayout->addRow(tr("Width:"), outWidthSpinBox_);
    outHeightSpinBox_ = new QSpinBox();
    outHeightSpinBox_->setRange(1,100000);
    outHeightSpinBox_->setValue(720);
    outHeightSpinBox_->setMaximumWidth(60);
    outWidthHeightLayout->addRow(tr("Height:"), outHeightSpinBox_);
    preserveAspectRatioCheckBox_ = createCheckBox(
        tr("Preserve canvas aspect ratio"), true, rasterSettingsLayout);

    // View Settings
    useViewSettings_ = createCheckBox(tr("Use view settings"), false, rasterSettingsLayout);

    // Motion Blur
    motionBlurCheckBox_ = createCheckBox(tr("Motion blur"), false, rasterSettingsLayout);
    motionBlurOptionsLayout_ = new QFormLayout();
    rasterSettingsLayout->addLayout(motionBlurOptionsLayout_);
    motionBlurNumSamplesSpinBox_ = new QSpinBox();
    motionBlurNumSamplesSpinBox_->setRange(1,100000);
    motionBlurNumSamplesSpinBox_->setValue(16);
    motionBlurNumSamplesSpinBox_->setMaximumWidth(60);
    motionBlurOptionsLayout_->addRow(tr("        number of samples:"), motionBlurNumSamplesSpinBox_);

    // Hide motion blur options until checked
    onMotionBlurChanged_(motionBlurCheckBox_->isChecked());

    // Vector settings
    vectorSettingsBox_ = new QGroupBox(tr("Options"));
    QVBoxLayout * vectorSettingsLayout = new QVBoxLayout();
    vectorSettingsBox_->setLayout(vectorSettingsLayout);

    // Background as rect
    backgroundAsRect_ = createCheckBox(
        tr("Export background color and/or images as rectangle shapes"),
        false, vectorSettingsLayout);

    // Fill variable-width strokes
    fillVariableWidthStrokes_ = createCheckBox(
        tr("Export variable-width strokes as filled paths"),
        false, vectorSettingsLayout);

    // Export/Cancel dialog buttons
    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel);
    buttonBox->addButton(tr("Export"), QDialogButtonBox::AcceptRole);

    // Main layout
    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget(outputFilesGroupBox);
    layout->addWidget(rasterSettingsBox_);
    layout->addWidget(vectorSettingsBox_);
    layout->addStretch();
    layout->addWidget(buttonBox);
    setLayout(layout);

    // Set initial widget values
    updateFilename_();
    updateDialogFromScene();
    onFileTypeChanged_();

    // By default, set focus to the dialog, so that pressing Enter
    // does the export.
    setFocus();

    // Create connections
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    connect(scene_, SIGNAL(changed()), this, SLOT(updateDialogFromScene()));

    connect(outWidthSpinBox_, SIGNAL(valueChanged(int)), this, SLOT(onOutWidthChanged_(int)));
    connect(outHeightSpinBox_, SIGNAL(valueChanged(int)), this, SLOT(onOutHeightChanged_(int)));
    connect(preserveAspectRatioCheckBox_, SIGNAL(toggled(bool)), this, SLOT(onPreserveAspectRatioChanged_(bool)));
    connect(motionBlurCheckBox_, SIGNAL(toggled(bool)), this, SLOT(onMotionBlurChanged_(bool)));
}

void ExportAsDialog::setVisible(bool visible)
{
    if(visible)
    {
        enforcePngAspectRatio_();
    }

    QDialog::setVisible(visible);
}

Scene * ExportAsDialog::scene() const
{
    return scene_;
}

int ExportAsDialog::numFileTypes() const
{
    const auto& fileTypes = exportFileTypes();
    return static_cast<int>(fileTypes.size());
}

const ExportFileTypeInfo* ExportAsDialog::fileTypeInfo(int i) const
{
    const auto& fileTypes = exportFileTypes();
    int n = static_cast<int>(fileTypes.size());
    if (i >= 0 && i < n) {
        return &fileTypes[i];
    }
    else {
        return nullptr;
    }
}

const ExportFileTypeInfo* ExportAsDialog::fileTypeInfo() const
{
    return fileTypeInfo(fileFormatComboBox_->currentIndex());
}

int ExportAsDialog::outWidth() const
{
    return outWidthSpinBox_->value();
}

int ExportAsDialog::outHeight() const
{
    return outHeightSpinBox_->value();
}

bool ExportAsDialog::preserveAspectRatio() const
{
    return preserveAspectRatioCheckBox_->isChecked();
}

bool ExportAsDialog::useViewSettings() const
{
    return useViewSettings_->isChecked();
}

bool ExportAsDialog::motionBlur() const
{
    return motionBlurCheckBox_->isChecked();
}

int ExportAsDialog::motionBlurNumSamples() const
{
    return motionBlurNumSamplesSpinBox_->value();
}

void ExportAsDialog::setPngWidthForHeight_()
{
    int sw = scene()->width();
    int sh = scene()->height();
    ignoreWidthHeightChanged_ = true; // prevent infinite recursion with pngWidthChanged()
    if(sh > 0)
        outWidthSpinBox_->setValue(sw * outHeight() / sh);
    ignoreWidthHeightChanged_ = false;
}

void ExportAsDialog::setPngHeightForWidth_()
{
    int sw = scene()->width();
    int sh = scene()->height();
    ignoreWidthHeightChanged_ = true; // prevent infinite recursion with pngHeightChanged()
    if(sw > 0)
        outHeightSpinBox_->setValue(sh * outWidth() / sw);
    ignoreWidthHeightChanged_ = false;
}

void ExportAsDialog::enforcePngAspectRatio_()
{
    if(preserveAspectRatio())
    {
        if(outWidth() > outHeight())
            setPngHeightForWidth_();
        else
            setPngWidthForHeight_();
    }
}

void ExportAsDialog::updateFilenameFromDocumentName() {
    updateFilename_();
}

void ExportAsDialog::showEvent(QShowEvent *event)
{
    // Give focus to the dialog itself by default, instead of its first child widget.
    // This is important for keyPressEvent() (see below).
    QWidget::showEvent(event);
    setFocus();
}

// https://forum.qt.io/topic/100296/qlineedit-returnpressed-closes-dialog
void ExportAsDialog::keyPressEvent(QKeyEvent *event)
{
    int key = event->key();
    if (focusWidget() != this && (key == Qt::Key_Return || key == Qt::Key_Enter))
    {
        event->ignore();
    }
    else
    {
        QDialog::keyPressEvent(event);
    }
}

void ExportAsDialog::onFileTypeChanged_()
{
    updateFilename_();

    const ExportFileTypeInfo* info = this->fileTypeInfo();
    if (info) {
        switch (info->category()) {
        case ExportFileTypeCategory::RasterImage:
            vectorSettingsBox_->hide();
            rasterSettingsBox_->show();
            break;
        case ExportFileTypeCategory::VectorImage:
            rasterSettingsBox_->hide();
            vectorSettingsBox_->show();
            break;
        }
    }
}

void ExportAsDialog::onFilenameLineEditEditingFinished_()
{
    bool isManualEdit = true;
    updateFilename_(isManualEdit);
}

namespace
{
class InconsistentFileNamesDialog: public QDialog
{
public:
    InconsistentFileNamesDialog(QWidget * parent = 0) :
        QDialog(parent)
    {
        // Window title
        setWindowTitle("Inconsistent file names");

        // Label with warning message and pattern
        label_ = new QLabel;
        label_->setWordWrap(true);

        // Text edit with list of inconsistent file names
        textEdit_ = new QTextEdit;
        textEdit_->setReadOnly(true);

        // Button box
        QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
        connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));

        // Layout
        QVBoxLayout * layout = new QVBoxLayout(this);
        layout->addWidget(label_);
        layout->addWidget(textEdit_);
        layout->addWidget(buttonBox);
        setLayout(layout);
    }

    void setPattern(const QString & pattern)
    {
        label_->setText(
            tr("Warning: The selected files do not have consistent names. "
               "The detected pattern is \"%1\", but the following files "
               "do not match it and therefore will be ignored:").arg(pattern));
    }

    void setFileNames(const QStringList & filenames)
    {
        QString text;
        for (int i=0; i<filenames.size(); ++i)
        {
            if (i>0)
                text += '\n';
            text += filenames[i];
        }
        textEdit_->setText(text);
    }

private:
    QLabel * label_;
    QTextEdit * textEdit_;
};

// Converts a list of file paths with numbers to one file path with
// a '*'.
//
// Example:
//
// Input: "hello01.png", "hello02.png"
// Output: "hello*.png"
//
QString getWilcardedFilePath(const QStringList & filenames, QWidget* errorDialogParentWidget)
{
    if (filenames.size() == 0)
    {
        return QString();
    }
    else if (filenames.size() == 1)
    {
        return filenames[0];
    }

    // Compute largest shared prefix of first two filenames
    const QString & s0 = filenames[0];
    const QString & s1 = filenames[1];
    int prefixLength = 0;
    while (s0.length() > prefixLength &&
           s1.length() > prefixLength &&
           s0[prefixLength] == s1[prefixLength])
    {
        prefixLength++;
    }

    // Chop digits at the end of prefix
    while (prefixLength > 0 &&
           s0[prefixLength-1].isDigit())
    {
        prefixLength--;
    }

    // Chop minus sign, unless all filenames have one, in which case it's
    // probably intented to be a separating dash and not a minus sign
    if (prefixLength > 0 && s0[prefixLength-1] == '-')
    {
        bool theyAllHaveIt = true;
        for (int i=0; i<filenames.size(); ++i)
        {
            if ( (filenames[i].length() < prefixLength) ||
                (filenames[i][prefixLength-1] != '-' )    )
            {
                theyAllHaveIt = false;
                break;
            }
        }

        if (!theyAllHaveIt)
            prefixLength--;
    }

    // Read wildcard of s0
    int s0WildcardLength = 0;
    if (s0.length() == prefixLength)
    {
        // That's weird, but might be the fallback value with
        // a wildcard at the end (i.e., without file extension)
        s0WildcardLength = 0;
    }
    else if (s0[prefixLength] == '-')
    {
        // s0 wildcard is negative
        s0WildcardLength++;
        while (s0.length() > prefixLength+s0WildcardLength &&
               s0[prefixLength+s0WildcardLength].isDigit())
        {
            s0WildcardLength++;
        }
    }
    else if (s0[prefixLength].isDigit())
    {
        // s0 wildcard is positive
        while (s0.length() > prefixLength+s0WildcardLength &&
               s0[prefixLength+s0WildcardLength].isDigit())
        {
            s0WildcardLength++;
        }
    }
    else
    {
        // Might be the fallback value
        s0WildcardLength = 0;
    }

    // Deduce prefix and suffix
    int suffixLength = s0.length() - prefixLength - s0WildcardLength;
    QString prefix = s0.left(prefixLength);
    QString suffix = s0.right(suffixLength);

    // Set filePath
    QString filePath = prefix + "*" + suffix;

    // Check for inconsistent names
    QStringList inconsistentFilenames;
    for (int i=0; i<filenames.size(); ++i)
    {
        // Check that prefix and suffix match
        if (filenames[i].left(prefixLength)  != prefix ||
            filenames[i].right(suffixLength) != suffix )
        {
            inconsistentFilenames << filenames[i];
        }

        // Check that wildcard can be converted to an int
        else
        {
            // Get wildcard and convert to int
            QString w = filenames[i].mid(prefixLength, filenames[i].length() - filePath.length() + 1);
            bool canConvertToInt;
            w.toInt(&canConvertToInt);

            // Add to inconsistent names if it cannot be converted to an int
            // (unless length == 0, in which case it's the fallback value)
            if (w.length() > 0 && !canConvertToInt)
            {
                inconsistentFilenames << filenames[i];
            }
        }
    }

    // Display warning to user if some file names are inconsistent
    if (inconsistentFilenames.length() > 0)
    {
        // issue warning
        InconsistentFileNamesDialog warnDialog(errorDialogParentWidget);
        warnDialog.setPattern(filePath);
        warnDialog.setFileNames(inconsistentFilenames);
        warnDialog.exec();
    }

    return filePath;
}

}

void ExportAsDialog::onFilenameBrowseButtonClicked_()
{
    // Get filenames
    QDir documentDir = global()->documentDir();

    // Unfortunately, we need to choose between `QFileDialog::AnyFile`, which
    // can select files that don't exist yet, but only one, or
    // `QFileDialog::ExistingFiles`, which can select multiple files, but only
    // files that exist. We would like `AnyFileOrExistingFiles`, but since this
    // doesn't exist `AnyFile` is the more suitable for our case. This means
    // that there will in fact be only one file in `filenames`, so the whole
    // "wildcard detection" code path isn't actually used, but we keep it in
    // case we find a way to do `AnyFileOrExistingFiles` in the future.
    //
    // Also note that we do not use the convenient static function
    // `QFileDialog::getSaveFileName()`, because the static function doesn't
    // allow changing the name of the button from "Save" to "Select". Using a
    // button named "Save" would be very misleading for users: they may think
    // that the file is already exported after clicking the button.
    //
    QFileDialog dialog(this);
    dialog.setWindowTitle(tr("Select File Name"));
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setLabelText(QFileDialog::Accept, tr("Select"));
    dialog.setDirectory(documentDir.path());
    bool accepted = dialog.exec();
    if (!accepted) {
        return;
    }
    QStringList filePaths = dialog.selectedFiles();

    // Convert to path relative to current document
    for (int i = 0; i < filePaths.size(); ++i)
    {
        filePaths[i] = documentDir.relativeFilePath(filePaths[i]);
    }

    // Detect wildcard
    QString filePath = getWilcardedFilePath(filePaths, this);
    filenameLineEdit_->setText(filePath);
    bool isManualEdit = true;
    updateFilename_(isManualEdit);
}

void ExportAsDialog::onFrameRangeTypeChanged_() {
    updateFilename_();
}

namespace {

// QFileInfo doesn't have any convenient way for editing non-existing file
// paths (e.g., methods such as `setSuffix()`), so we do it ourself.
//
// Note: QFileInfo has the following behavior in case of trailing dots:
//
// path                  baseName    completeSuffix
//
// foo/test.tar.         test        tar.
//
// foo/test..            test        .
// foo/test.             test        (nothing)
// foo/test              test        (nothing)
//
// foo/..                (nothing)   .
// foo/.                 (nothing)   (nothing)
// foo/                  (nothing)   (nothing)
//
// Note how with just the baseName and completeSuffix, it is not possible to
// distinguish between `test.` and `test`. This means that with this design, we
// need to somehow store this extra information.
//
// In contrast, std::filesystem::path has IMHO the better design, where the
// leading dot is considered part of the extension, so that user can explicitly
// set replace_extension(".zip") vs replace_extension(".") vs
// replace_extension(""). See:
// https://en.cppreference.com/w/cpp/filesystem/path/replace_extension
//
// However, std::filesystem::path doesn't have the concept of "complete
// extension". It would be nice but for now we only have svg and png,
// nothing like "tar.gz", so we use the same API as std::filesystem::path
// so that it's easier to migrate to std::filesystem::path later.
//
struct FilePath {

    FilePath(const QString & path) {

        // Extract prefix, that is everything before the filename.
        //
        // On Windows, both `/` and `\` are considered path separators.
        //
        // On Linux/macSO, only `/` is considered a path separator, and `\` can
        // be used as normal character.
        //
        int i = path.lastIndexOf('/'); // Returns -1 if not found
//#ifdef Q_OS_WINDOWS
        i = (std::max)(i, path.lastIndexOf('\\'));
//#endif
        prefix_ = path.left(i + 1);         // OK if i == -1
        QString filename = path.mid(i + 1); // OK if i == -1

        // Decompose filename in stem and extension
        //
        // https://en.cppreference.com/w/cpp/filesystem/path/extension
        //
        // Path                     Extension
        //
        // "/foo/bar.txt"           ".txt"
        // "/foo/bar."              "."
        // "/foo/bar"               ""
        // "/foo/bar.txt/bar.cc"    ".cc"
        // "/foo/bar.txt/bar."      "."
        // "/foo/bar.txt/bar"       ""
        // "/foo/."                 ""
        // "/foo/.."                ""
        // "/foo/.hidden"           ""
        // "/foo/..bar"             ".bar"
        //
        int j = filename.lastIndexOf('.');
        if (j == -1 || j == 0 || filename == "." || filename == "..") {
            stem_ = filename;
            extension_ = "";
        }
        else {
            stem_ = filename.left(j);
            extension_ = filename.mid(j);
        }
    }

    /// Converts the file path to a string.
    ///
    QString toString() {
        return prefix_ + stem_ + extension_;
    }

    /// Returns the part of the path before the filename.
    ///
    QString prefix() const {
        return prefix_;
    }

    /// Returns the stem.
    ///
    QString stem() const {
        return stem_;
    }

    /// Returns the extension, including the leading dot if any.
    ///
    QString extension() const {
        return extension_;
    }

    /// Returns the extension, not including the leading dot (if any).
    ///
    QString extensionWithoutLeadingDot() const {
        QString res = extension_;
        if (res.startsWith(".")) {
            res.remove(0, 1); // Remove first character
        }
        return res;
    }

    /// Replaces the extension. Automatically adds a leading
    /// dot if the given `extension` doesn't start with one.
    ///
    void replaceExtension(const QString & extension) {
        if (!extension.isEmpty() && extension.front() != '.') {
            extension_ = "." + extension;
        }
        else {
            extension_ = extension;
        }
    }

    /// Replaces the stem.
    ///
    void replaceStem(const QString & stem) {
        stem_ = stem;
    }

private:
    QString prefix_;    // `some/dir/`
    QString stem_;      // `myfile.tar`
    QString extension_; // `.gz`
};

}

void ExportAsDialog::updateFilename_(bool isManualEdit)
{
    FilePath path(filenameLineEdit_->text());
    QString stem = path.stem();

    // Determine whether the new exprt filename is an explicit filename that
    // shouldn't be changed even after doing a "Save As". For now we only check
    // whether the export name is exactly equal (except for '*') to the
    // document name. In the future, we may want to do smarter pattern
    // matching, e.g.:
    //
    //    previous        previous         new             automatic new
    //    document name   export path      document name   export path
    //
    //    myFile          out/myFile - *   myOtherFile     out/myOtherFile - *
    //
    if (isManualEdit) {
        if (stem.isEmpty()) {
            hasExplicitExportFilename_ = false;
        }
        else {
            QString documentName = global()->documentName();
            QString stemWithoutStar = stem;
            stemWithoutStar.replace("*", "");
            if (stemWithoutStar == documentName) {
                hasExplicitExportFilename_ = false;
            }
            else {
                hasExplicitExportFilename_ = true;
            }
        }
    }

    // Auto-set the stem based on the document name if no explicit export
    // filename had previously been given.
    //
    if (hasExplicitExportFilename_ == false) {
        hasExplicitExportFilename_ = false;
        QString documentName = global()->documentName();
        if (documentName.isEmpty()) {
            stem = "unnamed";
        }
        else {
            stem = documentName;
        }
    }

    // Add '*' for image sequence formats.
    if (singleImage_->isChecked()) {
        stem.replace("*", "");
    }
    else {
        if (stem.indexOf('*') == -1) {
            stem += '*';
        }
    }
    path.replaceStem(stem);

    // Update extension.
    QString targetExtension = "svg";
    int targetTypeIndex = fileFormatComboBox_->currentIndex();
    const ExportFileTypeInfo* currentFileTypeInfo = this->fileTypeInfo();
    if (currentFileTypeInfo) {
        targetExtension = QString(currentFileTypeInfo->extension().data());
    }
    if (isManualEdit) {
        QString currentExtension = path.extensionWithoutLeadingDot();
        if (currentExtension != targetExtension) {
            // Set file type based on file path
            int n = numFileTypes();
            for (int i = 0; i < n; ++i) {
                const ExportFileTypeInfo* info  = this->fileTypeInfo(i);
                if (info && currentExtension == info->extension().data()) {
                    targetExtension = QString(info->extension().data());
                    targetTypeIndex = i;
                    break;
                }
            }
        }
    }
    path.replaceExtension(targetExtension);

    // Update file path.
    filenameLineEdit_->setText(path.toString());

    // Update combo box if needed. We do this last to ensure the path is
    // already updated in case we re-enter this function due to signals/slots.
    // This is especially important in case isManualEdit is true.
    //
    int currentIndex = fileFormatComboBox_->currentIndex();
    if (targetTypeIndex != currentIndex) {
        fileFormatComboBox_->setCurrentIndex(targetTypeIndex);
    }
}


void ExportAsDialog::onOutWidthChanged_(int )
{
    if(!ignoreWidthHeightChanged_ && preserveAspectRatio())
    {
        setPngHeightForWidth_();
    }
}

void ExportAsDialog::onOutHeightChanged_(int )
{
    if(!ignoreWidthHeightChanged_ && preserveAspectRatio())
    {
        setPngWidthForHeight_();
    }
}

void ExportAsDialog::onPreserveAspectRatioChanged_(bool )
{
    enforcePngAspectRatio_();
}

void ExportAsDialog::onMotionBlurChanged_(bool b)
{
    // Iterate over all widgets in motionBlurOptionsLayout_
    // and change their visibility.
    int n = motionBlurOptionsLayout_->count();
    for (int i = 0; i < n; ++i) {
        QLayoutItem * item = motionBlurOptionsLayout_->itemAt(i);
        if (item) {
             QWidget * widget = item->widget();
             if (widget) {
                 widget->setVisible(b);
             }
        }
    }
}

void ExportAsDialog::accept()
{
    QDialog::accept();
}

void ExportAsDialog::reject()
{
    QDialog::reject();
}

void ExportAsDialog::updateDialogFromScene()
{
    ignoreWidthHeightChanged_ = true;
    outWidthSpinBox_->setValue(scene()->width());
    outHeightSpinBox_->setValue(scene()->height());
    ignoreWidthHeightChanged_ = false;
}
