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

#include "BackgroundWidget.h"

#include "Background.h"
#include "BackgroundUrlValidator.h"
#include "../ColorSelector.h"
#include "../Global.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QFileDialog>
#include <QTextEdit>
#include <QDialogButtonBox>

BackgroundWidget::BackgroundWidget(QWidget * parent) :
    QWidget(parent),
    background_(nullptr),
    isUpdatingFromBackground_(false),
    isBeingEdited_(false)
{
    // Clarify that there is one background per layer
    QVBoxLayout * whichLayerAndFormLayout = new QVBoxLayout();
    setLayout(whichLayerAndFormLayout);
    whichLayerAndFormLayout->addWidget(new QLabel(tr("(Note: each layer has its own background)")));

    // Form layout (contains everything except the clarification note above)
    QFormLayout * layout = new QFormLayout();
    whichLayerAndFormLayout->addLayout(layout);
    whichLayerAndFormLayout->addStretch();

    // Color
    colorSelector_ = new ColorSelector(Qt::transparent);
    colorSelector_->setToolTip(tr("Set background color"));
    colorSelector_->setStatusTip(tr("Set background color, possibly transparent."));
    layout->addRow(tr("Color:"), colorSelector_);
    connect(colorSelector_, SIGNAL(colorChanged(Color)),
            this, SLOT(processColorSelectorColorChanged_(Color)));

    // Images
    imageLineEdit_ = new QLineEdit();
    imageLineEdit_->setValidator(new BackgroundUrlValidator(imageLineEdit_));
    imageLineEdit_->setToolTip(tr("Set background image(s) url\n\n"
                                  "Example 1: 'image.png' for the same image at all frames\n"
                                  "Example 2: 'image*.png' for 'image2.png' on frame 2, etc."));
    imageLineEdit_->setStatusTip(tr("Set background image(s) url. For example, set "
                                    "'image.png' for a fixed image shared across all frames, "
                                    ", or set 'image*.png' for 'image1.png' at frame 1, "
                                    "'image2.png' at frame 2, etc. Paths must be relative to "
                                    "where the vec file is saved."));
    imageBrowseButton_ = new QPushButton("...");
    imageBrowseButton_->setToolTip(tr("Browse for background image(s)"));
    imageBrowseButton_->setStatusTip(tr("Browse for background image(s). Select two or more files, "
                                        "and a pattern of the form 'image*.png' will be automatically "
                                        "detected, loading all images matching patterns even if not selected."));
    imageBrowseButton_->setMaximumWidth(30);
    imageRefreshButton_ = new QPushButton(QIcon(":/images/refresh.png"), tr(""));
    imageRefreshButton_->setToolTip(tr("Reload background image(s)"));
    imageRefreshButton_->setStatusTip(tr("Reload background image(s) to reflect changes on disk."));
    imageRefreshButton_->setMaximumWidth(30);
    QHBoxLayout * imagesLayout = new QHBoxLayout();
#ifdef Q_OS_MAC
    imagesLayout->setSpacing(10);
#else
    imagesLayout->setSpacing(0);
#endif
    imagesLayout->addWidget(imageLineEdit_);
    imagesLayout->addWidget(imageBrowseButton_);
    imagesLayout->addWidget(imageRefreshButton_);
    layout->addRow(tr("Image(s):"), imagesLayout);
    connect(imageLineEdit_, SIGNAL(editingFinished()),
            this, SLOT(processImageLineEditEditingFinished_()));
    connect(imageBrowseButton_, SIGNAL(clicked(bool)),
            this, SLOT(processImageBrowseButtonClicked_()));
    connect(imageRefreshButton_, SIGNAL(clicked(bool)),
            this, SLOT(processImageRefreshButtonClicked_()));

    // Position
    leftSpinBox_ = new QDoubleSpinBox();
    leftSpinBox_->setToolTip(tr("X coordinate of top-left corner of background image(s)"));
    leftSpinBox_->setStatusTip(tr("Set the X coordinate of the position of the top-left corner of background image(s)."));
    leftSpinBox_->setMaximumWidth(80);
    leftSpinBox_->setMinimum(-1e6);
    leftSpinBox_->setMaximum(1e6);
    topSpinBox_ = new QDoubleSpinBox();
    topSpinBox_->setToolTip(tr("Y coordinate of top-left corner of background image(s)"));
    topSpinBox_->setStatusTip(tr("Set the Y coordinate of the position of the top-left corner of background image(s)."));
    topSpinBox_->setMaximumWidth(80);
    topSpinBox_->setMinimum(-1e6);
    topSpinBox_->setMaximum(1e6);
    QHBoxLayout * positionLayout = new QHBoxLayout();
    positionLayout->addWidget(leftSpinBox_);
    positionLayout->addWidget(topSpinBox_);
    layout->addRow(tr("Position:"), positionLayout);
    connect(leftSpinBox_, SIGNAL(valueChanged(double)),
            this, SLOT(processLeftSpinBoxValueChanged_(double)));
    connect(topSpinBox_, SIGNAL(valueChanged(double)),
            this, SLOT(processTopSpinBoxValueChanged_(double)));
    connect(leftSpinBox_, SIGNAL(editingFinished()),
            this, SLOT(processLeftSpinBoxEditingFinished_()));
    connect(topSpinBox_, SIGNAL(editingFinished()),
            this, SLOT(processTopSpinBoxEditingFinished_()));

    // Size
    sizeComboBox_ = new QComboBox();
    sizeComboBox_->setToolTip(tr("Set size of background image(s)"));
    sizeComboBox_->setStatusTip(tr("Set the size of background image(s)."));
    sizeComboBox_->addItem(tr("Fit to canvas"));
    sizeComboBox_->addItem(tr("Manual"));
    sizeComboBox_->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));
    widthSpinBox_ = new QDoubleSpinBox();
    widthSpinBox_->setToolTip(tr("Width of background image(s)"));
    widthSpinBox_->setStatusTip(tr("Set width of background image(s)."));
    widthSpinBox_->setMaximumWidth(80);
    widthSpinBox_->setMinimum(-1e6);
    widthSpinBox_->setMaximum(1e6);
    widthSpinBox_->setValue(1280);
    heightSpinBox_ = new QDoubleSpinBox();
    heightSpinBox_->setToolTip(tr("Height of background image(s)"));
    heightSpinBox_->setStatusTip(tr("Set height of background image(s)."));
    heightSpinBox_->setMaximumWidth(80);
    heightSpinBox_->setMinimum(-1e6);
    heightSpinBox_->setMaximum(1e6);
    heightSpinBox_->setValue(720);
    QGridLayout * sizeLayout = new QGridLayout();
    sizeLayout->addWidget(sizeComboBox_, 0, 0, 1, 2);
    sizeLayout->addWidget(widthSpinBox_, 1, 0);
    sizeLayout->addWidget(heightSpinBox_, 1, 1);
    layout->addRow(tr("Size:"), sizeLayout);
    connect(sizeComboBox_, SIGNAL(currentIndexChanged(int)),
            this, SLOT(processSizeComboBoxCurrentIndexChanged_(int)));
    connect(widthSpinBox_, SIGNAL(valueChanged(double)),
            this, SLOT(processWidthSpinBoxValueChanged_(double)));
    connect(heightSpinBox_, SIGNAL(valueChanged(double)),
            this, SLOT(processHeightSpinBoxValueChanged_(double)));
    connect(widthSpinBox_, SIGNAL(editingFinished()),
            this, SLOT(processWidthSpinBoxEditingFinished_()));
    connect(heightSpinBox_, SIGNAL(editingFinished()),
            this, SLOT(processHeightSpinBoxEditingFinished_()));

    // Repeat
    repeatComboBox_ = new QComboBox();
    repeatComboBox_->setToolTip(tr("Repeat background image(s)"));
    repeatComboBox_->setStatusTip(tr("Set whether background image(s) should "
                                     "be repeated, either horizontally, vertically, or both"));
    repeatComboBox_->addItem(tr("No"));
    repeatComboBox_->addItem(tr("Horizontally"));
    repeatComboBox_->addItem(tr("Vertically"));
    repeatComboBox_->addItem(tr("Both"));
    repeatComboBox_->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));
    layout->addRow(tr("Repeat:"), repeatComboBox_);
    connect(repeatComboBox_, SIGNAL(currentIndexChanged(int)),
            this, SLOT(processRepeatComboBoxCurrentIndexChanged_(int)));

    // Opacity
    opacitySpinBox_ = new QDoubleSpinBox();
    opacitySpinBox_->setToolTip(tr("Opacity of background image(s)"));
    opacitySpinBox_->setStatusTip(tr("Set the opacity of background image(s). Note: this does "
                                     "not affect the opacity of the background color (use an alpha "
                                     "value for the color instead)."));
    opacitySpinBox_->setMaximumWidth(80);
    opacitySpinBox_->setMinimum(0);
    opacitySpinBox_->setMaximum(1);
    opacitySpinBox_->setSingleStep(0.1);
    opacitySpinBox_->setValue(1.0);
    layout->addRow(tr("Opacity:"), opacitySpinBox_);
    connect(opacitySpinBox_, SIGNAL(valueChanged(double)),
            this, SLOT(processOpacitySpinBoxValueChanged_(double)));
    connect(opacitySpinBox_, SIGNAL(editingFinished()),
            this, SLOT(processOpacitySpinBoxEditingFinished_()));

    // Hold
    holdCheckBox_ = new QCheckBox();
    holdCheckBox_->setToolTip(tr("Hold background image(s)"));
    holdCheckBox_->setStatusTip(tr("Set whether to hold background image(s). Example: 'image*.png'"
                                   " with only 'image01.png' and 'image03.png' on disk. At "
                                   "frame 2, if hold is checked, 'image01.png' appears. If hold is "
                                   "not checked, no image appears, unless 'image.png' exists in which "
                                   "case it is used as a fallback value."));
    holdCheckBox_->setChecked(true );
    layout->addRow(tr("Hold:"), holdCheckBox_);
    connect(holdCheckBox_, SIGNAL(toggled(bool)),
            this, SLOT(processHoldCheckBoxToggled_(bool)));

    // Set no background
    setBackground(nullptr);
}

void BackgroundWidget::setBackground(Background * background)
{
    // Disconnect previous connections. This assumes that background_
    // is either nullptr or not destroyed yet, which is ensured by
    // onBackgroundDestroyed_().
    if (background_)
    {
        background_->disconnect(this);
    }

    // Store value
    background_ = background;

    // Disable all widgets if background_ == NULL
    bool areChildrenEnabled = background_ ? true : false;
    QList<QWidget*> children = findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
    for (QWidget * w: children)
        w->setEnabled(areChildrenEnabled);

    // Set widgets values from background values
    updateFromBackground_();

    // Update widget values when data changes
    if (background_)
    {
        connect(background_, SIGNAL(destroyed()), this, SLOT(onBackgroundDestroyed_()));
        connect(background_, SIGNAL(changed()), this, SLOT(updateFromBackground_()));
    }
}

void BackgroundWidget::updateFromBackground_()
{
    if (background_)
    {
        // Set guard
        isUpdatingFromBackground_ = true;

        // Color
        colorSelector_->setColor(background_->color());

        // Image
        imageLineEdit_->setText(background_->imageUrl());

        // Position
        leftSpinBox_->setValue(background_->position()[0]);
        topSpinBox_->setValue(background_->position()[1]);

        // Size
        sizeComboBox_->setCurrentIndex((int) background_->sizeType());
        widthSpinBox_->setValue(background_->size()[0]);
        heightSpinBox_->setValue(background_->size()[1]);
        switch (background_->sizeType())
        {
        case Background::SizeType::Cover:
            widthSpinBox_->hide();
            heightSpinBox_->hide();
            break;
        case Background::SizeType::Manual:
            widthSpinBox_->show();
            heightSpinBox_->show();
            break;
        }

        // Repeat
        repeatComboBox_->setCurrentIndex((int) background_->repeatType());

        // Opacity
        opacitySpinBox_->setValue(background_->opacity());

        // Hold
        holdCheckBox_->setChecked(background_->hold());

        // Cache value before editing
        if (!isBeingEdited_)
        {
            dataBeforeEditing_ = background_->data();
        }

        // Unset guard
        isUpdatingFromBackground_ = false;
    }
}

void BackgroundWidget::onBackgroundDestroyed_()
{
    background_ = nullptr;
    setBackground(nullptr);
}

Background * BackgroundWidget::background() const
{
    return background_;
}

void BackgroundWidget::processColorSelectorColorChanged_(const Color & newColor)
{
    if (background_ && !isUpdatingFromBackground_)
    {
        isBeingEdited_ = true;
        background_->setColor(newColor);
        isBeingEdited_ = false;
        emitCheckpoint_();
    }
}

void BackgroundWidget::processImageLineEditEditingFinished_()
{
    if (background_ && !isUpdatingFromBackground_)
    {
        isBeingEdited_ = true;
        background_->setImageUrl(imageLineEdit_->text());
        isBeingEdited_ = false;
        emitCheckpoint_();
    }
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

void BackgroundWidget::processImageBrowseButtonClicked_()
{
    // Get filenames
    QDir documentDir = global()->documentDir();
    QStringList filenames = QFileDialog::getOpenFileNames(
                this,
                tr("Select image, or sequence of images, to set as background"),
                documentDir.path(),
                tr("Image files (*.jpg *.png)"));


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
            InconsistentFileNamesDialog dialog(this);
            dialog.setPattern(url);
            dialog.setFileNames(inconsistentFilenames);
            dialog.exec();
        }
    }

    // Set image url
    if (background_ && !isUpdatingFromBackground_)
    {
        isBeingEdited_ = true;
        background_->setImageUrl(url);
        isBeingEdited_ = false;
        emitCheckpoint_();
    }
}

void BackgroundWidget::processImageRefreshButtonClicked_()
{
    if (background_)
    {
        background_->clearCache();
    }
}

void BackgroundWidget::processLeftSpinBoxValueChanged_(double newLeft)
{
    if (background_ && !isUpdatingFromBackground_)
    {
        isBeingEdited_ = true;
        double top = background_->position()[1];
        background_->setPosition(Eigen::Vector2d(newLeft, top));
        isBeingEdited_ = false;
    }
}

void BackgroundWidget::processTopSpinBoxValueChanged_(double newTop)
{
    if (background_ && !isUpdatingFromBackground_)
    {
        isBeingEdited_ = true;
        double left = background_->position()[0];
        background_->setPosition(Eigen::Vector2d(left, newTop));
        isBeingEdited_ = false;
    }
}

void BackgroundWidget::processLeftSpinBoxEditingFinished_()
{
    emitCheckpoint_();
}

void BackgroundWidget::processTopSpinBoxEditingFinished_()
{
    emitCheckpoint_();
}

void BackgroundWidget::processSizeComboBoxCurrentIndexChanged_(int newSizeType)
{
    if (background_ && !isUpdatingFromBackground_)
    {
        isBeingEdited_ = true;
        background_->setSizeType(static_cast<Background::SizeType>(newSizeType));
        isBeingEdited_ = false;
        emitCheckpoint_();
    }
}

void BackgroundWidget::processWidthSpinBoxValueChanged_(double newWidth)
{
    if (background_ && !isUpdatingFromBackground_)
    {
        isBeingEdited_ = true;
        double height = background_->size()[1];
        background_->setSize(Eigen::Vector2d(newWidth, height));
        isBeingEdited_ = false;
    }
}

void BackgroundWidget::processHeightSpinBoxValueChanged_(double newHeight)
{
    if (background_ && !isUpdatingFromBackground_)
    {
        isBeingEdited_ = true;
        double width = background_->size()[0];
        background_->setSize(Eigen::Vector2d(width, newHeight));
        isBeingEdited_ = false;
    }
}

void BackgroundWidget::processWidthSpinBoxEditingFinished_()
{
    emitCheckpoint_();
}

void BackgroundWidget::processHeightSpinBoxEditingFinished_()
{
    emitCheckpoint_();
}

void BackgroundWidget::processRepeatComboBoxCurrentIndexChanged_(int newRepeatType)
{
    if (background_ && !isUpdatingFromBackground_)
    {
        isBeingEdited_ = true;
        background_->setRepeatType(static_cast<Background::RepeatType>(newRepeatType));
        isBeingEdited_ = false;
        emitCheckpoint_();
    }
}

void BackgroundWidget::processOpacitySpinBoxValueChanged_(double newOpacity)
{
    if (background_ && !isUpdatingFromBackground_)
    {
        isBeingEdited_ = true;
        background_->setOpacity(newOpacity);
        isBeingEdited_ = false;
    }
}

void BackgroundWidget::processOpacitySpinBoxEditingFinished_()
{
    emitCheckpoint_();
}

void BackgroundWidget::processHoldCheckBoxToggled_(bool newHold)
{
    if (background_ && !isUpdatingFromBackground_)
    {
        isBeingEdited_ = true;
        background_->setHold(newHold);
        isBeingEdited_ = false;
        emitCheckpoint_();
    }
}

void BackgroundWidget::emitCheckpoint_()
{
    if (background_ && (background_->data() != dataBeforeEditing_))
    {
        dataBeforeEditing_ = background_->data();
        background_->emitCheckpoint();
    }
}
