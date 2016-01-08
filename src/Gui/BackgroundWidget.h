// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef BACKGROUNDWIDGET_H
#define BACKGROUNDWIDGET_H

#include <QWidget>

class Background;
class ColorSelector;

class QLineEdit;
class QPushButton;
class QCheckBox;
class QDoubleSpinBox;
class QComboBox;

/// BackgroundWidget is a widget that operate on a
/// Background object, to change is value in a graphical way
///
/// Usage:
/// // Create a background object
/// Background * background = new Background();
///
/// // Add to some layout a widget to modify the background object
/// BackgroundWidget * backgroundWidget = new BackgroundWidget();
/// backgroundWidget->setBackground(background);
/// someLayout->addWidget(backgroundWidget);
///

class BackgroundWidget: public QWidget
{
public:
    BackgroundWidget(QWidget * parent = 0);

    void setBackground(Background * background);
    Background * background() const;

private:
    // Background viewed by widget
    Background * background_;

    // GUI
    // Color
    ColorSelector * colorSelector_;
    // Images
    QLineEdit * imagesTextEdit_;
    QPushButton * imagesButton_;
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
};

#endif // BACKGROUNDWIDGET_H
