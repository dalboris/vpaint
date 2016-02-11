// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef BACKGROUNDWIDGET_H
#define BACKGROUNDWIDGET_H

#include <QWidget>

#include "Color.h"
#include "BackgroundData.h"

class Background;
class ColorSelector;

class QLineEdit;
class QPushButton;
class QCheckBox;
class QDoubleSpinBox;
class QComboBox;

/// \class BackgroundWidget
/// BackgroundWidget is a widget that allows users to change
/// the value of a Background object in a graphical way
///
/// Usage:
/// \code
/// Background * background = new Background();
/// BackgroundWidget * backgroundWidget = new BackgroundWidget();
/// backgroundWidget->setBackground(background);
/// \endcode

class BackgroundWidget: public QWidget
{
    Q_OBJECT

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    // Constructor
    BackgroundWidget(QWidget * parent = 0);

    // Set/Get which Background object is edited by the BackgroundWidget
    void setBackground(Background * background);
    Background * background() const;

private slots:
    // Update values from background
    void updateFromBackground_();

    // Process user interaction with widgets
    // Color
    void processColorSelectorColorChanged_(const Color & newColor);
    // Image
    void processImageLineEditEditingFinished_();
    void processImageBrowseButtonClicked_();
    void processImageRefreshButtonClicked_();
    // Position
    void processLeftSpinBoxValueChanged_(double newLeft);
    void processTopSpinBoxValueChanged_(double newTop);
    void processLeftSpinBoxEditingFinished_();
    void processTopSpinBoxEditingFinished_();
    // Size
    void processSizeComboBoxCurrentIndexChanged_(int newSizeType);
    void processWidthSpinBoxValueChanged_(double newWidth);
    void processHeightSpinBoxValueChanged_(double newHeight);
    void processWidthSpinBoxEditingFinished_();
    void processHeightSpinBoxEditingFinished_();
    // Repeat
    void processRepeatComboBoxCurrentIndexChanged_(int newRepeatType);
    // Opacity
    void processOpacitySpinBoxValueChanged_(double newOpacity);
    void processOpacitySpinBoxEditingFinished_();
    // Hold
    void processHoldCheckBoxToggled_(bool newHold);
    // Repeat
    void processRepeatCheckBoxToggled_(bool newRepeat);

private:
    // Background operated by BackgroundWidget
    Background * background_;

    // GUI
    // Color
    ColorSelector * colorSelector_;
    // Images
    QLineEdit * imageLineEdit_;
    QPushButton * imageBrowseButton_;
    QPushButton * imageRefreshButton_;
    // Position
    QDoubleSpinBox * leftSpinBox_;
    QDoubleSpinBox * topSpinBox_;
    // Size
    QComboBox * sizeComboBox_;
    QDoubleSpinBox * widthSpinBox_;
    QDoubleSpinBox * heightSpinBox_;
    // Repeat
    QComboBox * repeatComboBox_;
    // Opacity
    QDoubleSpinBox * opacitySpinBox_;
    // Hold
    QCheckBox * holdCheckBox_;
    // Repeat
    QCheckBox * repeatCheckBox_;

    // Guard needed for updateFromBackground_()
    // It is needed is to avoid modifying back 'this->background_' when
    // 'this' updates its widget values from 'this->background_'
    bool isUpdatingFromBackground_;

    // Mechanism for issuing undo commands
    // We keep a local copy of background data to compare values before and after
    // editing, in order to issue an undoable command only when they differ
    void emitCheckpoint_();
    bool isBeingEdited_;
    BackgroundData dataBeforeEditing_;
};

#endif // BACKGROUNDWIDGET_H
