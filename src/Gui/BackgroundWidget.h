// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef BACKGROUNDWIDGET_H
#define BACKGROUNDWIDGET_H

#include <QWidget>

#include "Color.h"

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
    // Constructor
    BackgroundWidget(QWidget * parent = 0);

    // Set/Get which Background object is edited by the BackgroundWidget
    void setBackground(Background * background);
    Background * background() const;

private slots:
    // Update values from background
    void updateFromBackground_();

    // Process user interaction with widgets
    void processColorSelectorColorChanged_(const Color & newColor);
    void processImageLineEditTextChanged_(const QString & newText);
    void processImageBrowseButtonClicked_();
    void processImageRefreshButtonClicked_();
    void processLeftSpinBoxValueChanged_(double newLeft);
    void processTopSpinBoxValueChanged_(double newTop);
    void processSizeComboBoxCurrentIndexChanged_(int newSizeType);
    void processWidthSpinBoxValueChanged_(double newWidth);
    void processHeightSpinBoxValueChanged_(double newHeight);
    void processRepeatComboBoxCurrentIndexChanged_(int newRepeatType);
    void processOpacitySpinBoxValueChanged_(double newOpacity);
    void processHoldCheckBoxToggled_(bool newHold);

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

    // Guard needed for updateFromBackground_()
    // It is needed is to avoid modifying back 'this->background_' when
    // 'this' updates its widget values from 'this->background_'
    bool isUpdatingFromBackground_;
};

#endif // BACKGROUNDWIDGET_H
