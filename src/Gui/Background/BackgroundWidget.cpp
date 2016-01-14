// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2016 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "BackgroundWidget.h"
#include "Background.h"
#include "ColorSelector.h"
#include "../Global.h" // XXX This is for documentDir()

#include <QFormLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QFileDialog>
#include <QMessageBox>

BackgroundWidget::BackgroundWidget(QWidget * parent) :
    QWidget(parent),
    background_(0),
    isUpdatingFromBackground_(false),
    isBeingEdited_(false)
{
    // Layout
    QFormLayout * layout = new QFormLayout();
    setLayout(layout);

    // Color
    colorSelector_ = new ColorSelector(Qt::white);
    layout->addRow(tr("Color:"), colorSelector_);
    connect(colorSelector_, SIGNAL(colorChanged(Color)),
            this, SLOT(processColorSelectorColorChanged_(Color)));

    // Images
    imageLineEdit_ = new QLineEdit();
    imageBrowseButton_ = new QPushButton("...");
    imageBrowseButton_->setMaximumWidth(30);
    imageRefreshButton_ = new QPushButton("O"); // XXX set icon
    imageRefreshButton_->setMaximumWidth(30);
    QHBoxLayout * imagesLayout = new QHBoxLayout();
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
    leftSpinBox_->setMaximumWidth(80);
    leftSpinBox_->setMinimum(-1e6);
    leftSpinBox_->setMaximum(1e6);
    topSpinBox_ = new QDoubleSpinBox();
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
    sizeComboBox_->addItem(tr("Fit to canvas"));
    sizeComboBox_->addItem(tr("Manual"));
    sizeComboBox_->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));
    widthSpinBox_ = new QDoubleSpinBox();
    widthSpinBox_->setMaximumWidth(80);
    widthSpinBox_->setMinimum(-1e6);
    widthSpinBox_->setMaximum(1e6);
    widthSpinBox_->setValue(1280);
    heightSpinBox_ = new QDoubleSpinBox();
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
    holdCheckBox_->setChecked(true );
    layout->addRow(tr("Hold:"), holdCheckBox_);
    connect(holdCheckBox_, SIGNAL(toggled(bool)),
            this, SLOT(processHoldCheckBoxToggled_(bool)));

    // Set no background
    setBackground(0);
}

void BackgroundWidget::setBackground(Background * background)
{
    // Delete previous connections
    if (background_)
    {
        background_->disconnect(this);
    }

    // Store value
    background_ = background;

    // Disable all widgets if background_ == NULL
    bool areChildrenEnabled = background_ ? true : false;
    QList<QWidget*> children = findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
    foreach (QWidget * w, children)
        w->setEnabled(areChildrenEnabled);

    // Set widgets values from background values
    updateFromBackground_();

    // Create connections
    if (background_)
    {
        // XXX We could instead use individual values connections instead,
        // to avoid updating everything each time
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
        QString inconsistentFilenames;
        for (int i=0; i<filenames.size(); ++i)
        {
            bool inconsistent = false;

            if (filenames[i].left(prefixLength)  != prefix ||
                filenames[i].right(suffixLength) != suffix )
            {
                inconsistent = true;
            }
            else
            {
                // Get wildcard
                QString w = filenames[i];
                w.remove(0, prefixLength);
                w.chop(suffixLength);

                if (w.length() == 0)
                {
                    // It's the fallback value: filename[i] == prefix+suffix
                    inconsistent = false;
                }
                else
                {
                    // Try to convert to an int
                    bool ok;
                    w.toInt(&ok);
                    if (!ok)
                    {
                        inconsistent = true;
                    }
                }
            }

            if(inconsistent)
            {
                inconsistentFilenames += filenames[i] + "\n";

            }
        }
        if (inconsistentFilenames.length() > 0)
        {
            // Remove last newline
            inconsistentFilenames.chop(1);

            // issue warning
            QMessageBox::warning(
                        this,
                        tr("Inconsistent file names"),
                        tr("Warning: The selected files don't have a consistent naming scheme. "
                           "The following files do not match \"%1\" and will be ignored:\n%2")
                        .arg(url)
                        .arg(inconsistentFilenames));
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
