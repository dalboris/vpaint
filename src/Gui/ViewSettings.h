// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef VIEWSETTINGS_H
#define VIEWSETTINGS_H

#include "TimeDef.h"
#include <QWidget>
#include <QPushButton>

class ViewSettings
{
public:
    ViewSettings();
    ~ViewSettings();

    // Display
    double zoom() const;
    void setZoom(double newValue);

    enum DisplayMode {
        ILLUSTRATION,
        OUTLINE,
        ILLUSTRATION_OUTLINE
    };
    DisplayMode displayMode() const;
    void setDisplayMode(DisplayMode mode);
    void toggleOutline();
    void toggleOutlineOnly();

    bool drawCursor() const;
    void setDrawCursor(bool newValue);

    // XXX isMainDrawing is used to draw the rectangle of
    // selection only once, only in 2D view, and never when
    // exporting to an image. This is a hack. In the future, the
    // rectangle of selection shouldn't be drawn by VAC::draw(),
    // and this isMainDrawing() attribute should be removed.
    bool isMainDrawing() const;
    void setMainDrawing(bool newValue);


    int vertexTopologySize() const;
    void setVertexTopologySize(int newValue);

    int edgeTopologyWidth() const;
    void setEdgeTopologyWidth(int newValue);

    bool drawTopologyFaces() const;
    void setDrawTopologyFaces(bool newValue);


    bool screenRelative() const;
    void setScreenRelative(bool newValue);

    Time time() const;
    void setTime(const Time & t);

    // Onion Skinning

    bool onionSkinningIsEnabled() const;
    void setOnionSkinningIsEnabled(bool newValue);

    bool areOnionSkinsPickable() const;
    void setAreOnionSkinsPickable(bool newValue);

    int numOnionSkinsBefore() const;
    void setNumOnionSkinsBefore(int newValue);

    int numOnionSkinsAfter() const;
    void setNumOnionSkinsAfter(int newValue);

    Time onionSkinsTimeOffset() const;
    void setOnionSkinsTimeOffset(Time newValue);
    void setOnionSkinsTimeOffset(double newValue);

    double onionSkinsXOffset() const;
    void setOnionSkinsXOffset(double newValue);

    double onionSkinsYOffset() const;
    void setOnionSkinsYOffset(double newValue);

    double onionSkinsTransparencyRatio() const;
    void setOnionSkinsTransparencyRatio(double newValue);

private:
    // Display settings
    double zoom_;
    DisplayMode displayMode_;
    bool outlineOnly_;
    bool drawBackground_;
    bool drawCursor_;
    bool isMainDrawing_;
    int vertexTopologySize_;
    int edgeTopologyWidth_;
    bool drawTopologyFaces_;
    bool screenRelative_;
    Time time_;

    // Onion skinning
    bool onionSkinningIsEnabled_;
    bool areOnionSkinsPickable_;
    int numOnionSkinsBefore_;
    int numOnionSkinsAfter_;
    Time onionSkinsTimeOffset_;
    double onionSkinsXOffset_;
    double onionSkinsYOffset_;
    double onionSkinsTransparencyRatio_;
};

#include <QFormLayout>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QSlider>

class ViewSettingsWidget: public QWidget
{
    Q_OBJECT

public:
    ViewSettingsWidget(ViewSettings & viewSettings, QWidget * parent=0);
    ~ViewSettingsWidget();

    void setActive(bool isActive);
    void updateSettingsFromWidgetSilent();

signals:
    void changed();

public slots:
    void updateWidgetFromSettings();
    void updateSettingsFromWidget();
    void toggleVisible(bool checked);
    void processZoomValueChangedSignal(int n);

private slots:
    void incrFrame();
    void decrFrame();

private:
    int getFrame_();

    ViewSettings & viewSettings_;

    QSlider * vertexTopologySize_;
    QSlider * edgeTopologyWidth_;
    QCheckBox * drawTopologyFaces_;
    QCheckBox * screenRelative_;

    QCheckBox * onionSkinIsEnabled_;
    QCheckBox * areOnionSkinsPickable_;
    QSpinBox * numOnionSkinsBefore_;
    QSpinBox * numOnionSkinsAfter_;
    QDoubleSpinBox * onionSkinsTimeOffset_;
    QDoubleSpinBox * onionSkinsXOffset_;
    QDoubleSpinBox * onionSkinsYOffset_;
    QDoubleSpinBox * onionSkinsTransparencyRatio_;

    QPushButton * showHideSettingsButton_;
    QWidget * containerWidget;

    int buttonSize;

    QPushButton * displayModeButton_;
    QPushButton * displayModeButton_Normal_;
    QPushButton * displayModeButton_NormalOutline_;
    QPushButton * displayModeButton_Outline_;

    QPushButton * onionSkinningButton_;
    QPushButton * onionSkinningButton_Off_;
    QPushButton * onionSkinningButton_On_;

    bool ignoreZoomValueChangedSignal_;
    double zoomValue_; // We need this to remember value before rounding by the spinbox
    QSpinBox * zoomSpinBox_; // Note: using a QDOubleSpinBox doesn't solve the above issue,
                             //       and just introduce more hassle
    QLineEdit * frameLineEdit_;
};

#endif // VIEWSETTINGS_H
