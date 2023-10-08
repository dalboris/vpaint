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

const std::vector<ExportFileTypeInfo>& exportFileTypes() {
    static std::vector<ExportFileTypeInfo> types;
    using C = ExportFileTypeCategory;
    types.emplace_back("svg", "SVG Image", C::VectorImage);
    types.emplace_back("png", "PNG Image", C::RasterImage);
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

QCheckBox * createCheckBox(QString label, QLayout * layout)
{
    QCheckBox * res = new QCheckBox(label);
    res->setFocusPolicy(Qt::NoFocus);
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

    // Filename(s)
    filenameLineEdit_ = new QLineEdit();
    /*
    imageLineEdit_->setValidator(new BackgroundUrlValidator(imageLineEdit_));
    */
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
            this, SLOT(processFilenameLineEditEditingFinished_()));
    connect(filenameBrowseButton_, SIGNAL(clicked(bool)),
            this, SLOT(processFilenameBrowseButtonClicked_()));

    // Single image vs. Image sequence
    frameRangeGroup_ = new QButtonGroup();
    singleImage_ = createRadioButton(
        tr("Single image (current frame)"), frameRangeGroup_, outputFilesLayout);
    imageSequenceAll_ = createRadioButton(
        tr("Image sequence (all frames)"), frameRangeGroup_, outputFilesLayout);
    singleImage_->setChecked(true);

    // Image output size
    QGroupBox * outSizeGroupBox = new QGroupBox(tr("Image Output Size"));
    QVBoxLayout * outSizeLayout = new QVBoxLayout();
    outSizeGroupBox->setLayout(outSizeLayout);

    QFormLayout * outWidthHeightLayout = new QFormLayout();
    outSizeLayout->addLayout(outWidthHeightLayout);

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

    preserveAspectRatioCheckBox_ = createCheckBox(tr("Preserve canvas aspect ratio"), outSizeLayout);
    preserveAspectRatioCheckBox_->setChecked(true);

    // Rendering
    QGroupBox * renderingGroupBox = new QGroupBox(tr("Rendering"));
    QVBoxLayout * renderingLayout = new QVBoxLayout();
    renderingGroupBox->setLayout(renderingLayout);

    useViewSettings_ = createCheckBox(tr("Use view settings"), renderingLayout);
    useViewSettings_->setChecked(false);

    motionBlurCheckBox_ = createCheckBox(tr("Motion blur"), renderingLayout);
    motionBlurCheckBox_->setChecked(false);

    motionBlurOptionsLayout_ = new QFormLayout();
    renderingLayout->addLayout(motionBlurOptionsLayout_);

    motionBlurNumSamplesSpinBox_ = new QSpinBox();
    motionBlurNumSamplesSpinBox_->setRange(1,100000);
    motionBlurNumSamplesSpinBox_->setValue(16);
    motionBlurNumSamplesSpinBox_->setMaximumWidth(60);
    motionBlurOptionsLayout_->addRow(tr("        number of samples:"), motionBlurNumSamplesSpinBox_);

    // Hide motion blur options until checked
    processMotionBlurChanged_(motionBlurCheckBox_->isChecked());

    // Export/Cancel dialog buttons
    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel);
    buttonBox->addButton(tr("Export"), QDialogButtonBox::AcceptRole);

    // Main layout
    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget(outputFilesGroupBox);
    layout->addWidget(outSizeGroupBox);
    layout->addWidget(renderingGroupBox);
    layout->addStretch();
    layout->addWidget(buttonBox);
    setLayout(layout);


    // Set initial widget values
    updateDialogFromScene();

    // By default, set focus to the dialog, so that pressing Enter
    // does the export.
    setFocus();

    // Create connections
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    connect(scene_, SIGNAL(changed()), this, SLOT(updateDialogFromScene()));

    connect(outWidthSpinBox_, SIGNAL(valueChanged(int)), this, SLOT(processOutWidthChanged_(int)));
    connect(outHeightSpinBox_, SIGNAL(valueChanged(int)), this, SLOT(processOutHeightChanged_(int)));
    connect(preserveAspectRatioCheckBox_, SIGNAL(toggled(bool)), this, SLOT(processPreserveAspectRatioChanged_(bool)));
    connect(motionBlurCheckBox_, SIGNAL(toggled(bool)), this, SLOT(processMotionBlurChanged_(bool)));
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

const ExportFileTypeInfo* ExportAsDialog::fileTypeInfo() const
{
    const auto& fileTypes = exportFileTypes();
    int n = static_cast<int>(fileTypes.size());
    int i = fileFormatComboBox_->currentIndex();
    if (i >= 0 && i < n) {
        return &fileTypes[i];
    }
    else {
        return nullptr;
    }
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

void ExportAsDialog::processFilenameLineEditEditingFinished_()
{
    // TODO
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

    void setFileNames(const QStringList & fileNames)
    {
        QString text;
        for (int i=0; i<fileNames.size(); ++i)
        {
            if (i>0)
                text += '\n';
            text += fileNames[i];
        }
        textEdit_->setText(text);
    }

private:
    QLabel * label_;
    QTextEdit * textEdit_;
};
}

void ExportAsDialog::processFilenameBrowseButtonClicked_()
{
    // Get filenames
    QDir documentDir = global()->documentDir();

    // Unfortunately, we need to choose between `QFileDialog::AnyFile`, which
    // can select files that don't exist yet, but only one, or
    // `QFileDialog::ExistingFiles`, which can select multiple files, but only
    // files that exist. We would like `AnyFileOrExistingFiles`, but since this
    // doesn't exist `AnyFile` is the more suitable for our case. This means
    // that really, there will be only one file in `filenames`, so that the
    // whole "wildcard detection" code path isn't actually used, but we keep it
    // in case we find a way to do `AnyFileOrExistingFiles` in the future.
    //
    // Also, note that we do not use `QFileDialog::getSaveFileName()` so that
    // we can change the label of the accept button to "Select". Otherwise, the
    // label would be "Save", which could mislead the user into thinking that
    // the file is already exported after clicking the button.
    //
    QFileDialog dialog(this);
    dialog.setWindowTitle(tr("Select File Name"));
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setLabelText(QFileDialog::Accept, tr("Select"));
    dialog.setDirectory(documentDir.path());

    QStringList filenames;
    if (dialog.exec()) {
        filenames = dialog.selectedFiles();
    }

    // Convert to path relative to current document
    for (int i=0; i<filenames.size(); ++i)
    {
        filenames[i] = documentDir.relativeFilePath(filenames[i]);
    }

    // Detect wildcard
    QString url;
    if (filenames.size() == 0)
    {
        url = QString();
    }
    else if (filenames.size() == 1)
    {
        url = filenames[0];
    }
    else // filenames.size() >= 2
    {
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

        // Set url
        url = prefix + "*" + suffix;

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
                QString w = filenames[i].mid(prefixLength, filenames[i].length() - url.length() + 1);
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
            InconsistentFileNamesDialog warnDialog(this);
            warnDialog.setPattern(url);
            warnDialog.setFileNames(inconsistentFilenames);
            warnDialog.exec();
        }
    }

    filenameLineEdit_->setText(url);
}

void ExportAsDialog::updateFileName_() {
    // TODO
    /*
    QFileInfo fileInfo = filenameLineEdit_->text();
    const ExportFileTypeInfo* fileTypeInfo  = this->fileTypeInfo();
    QString targetExtension = "svg";
    if (fileTypeInfo) {
        targetExtension = QString(fileTypeInfo->extension().data());
    }
    if (text.isEmpty()) {
        newText = global()->documentName();
        // TODO: append extension
    }
    else {
        // TODO: check if extension is ok, else change it
    }
    */

}


void ExportAsDialog::processOutWidthChanged_(int )
{
    if(!ignoreWidthHeightChanged_ && preserveAspectRatio())
    {
        setPngHeightForWidth_();
    }
}

void ExportAsDialog::processOutHeightChanged_(int )
{
    if(!ignoreWidthHeightChanged_ && preserveAspectRatio())
    {
        setPngWidthForHeight_();
    }
}

void ExportAsDialog::processPreserveAspectRatioChanged_(bool )
{
    enforcePngAspectRatio_();
}

void ExportAsDialog::processMotionBlurChanged_(bool b)
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
