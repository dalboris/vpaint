// Copyright (C) 2012-2018 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef LAYERSWIDGET_H
#define LAYERSWIDGET_H

#include <vector>

#include <QWidget>

class QCheckBox;
class QLabel;
class QVBoxLayout;

/// One individual layer row in the Layers panel.
///
class LayerWidget: public QWidget
{
    Q_OBJECT

public:
    LayerWidget(int index, bool isCurrent = false);
    ~LayerWidget();

    int index() const;
    void setIndex(int index);

    bool isCurrent() const;
    void setCurrent(bool b);

protected:
    void mousePressEvent(QMouseEvent* event) override;

signals:
    // This signal is emitted when the user
    // requested to make this layer current.
    void requestCurrent(int layerIndex);

private:
    int index_;
    bool isCurrent_;
    QCheckBox * checkBox_;
    QLabel * label_;
    void updateBackground_();
    void updateCheckBoxState_();
    void updateLabelText_();
};

/// The whole Layers panel.
///
class LayersWidget: public QWidget
{
    Q_OBJECT

public:
    LayersWidget();
    ~LayersWidget();

private slots:
    void onRequestCurrent_(int index);

private:
    // Each LayerWidget is responsible for displaying info
    // about a given layer. When reordering the layers, the
    // LayerWidget instances are not reordered, but simply
    // assigned a different layer to display info of.
    void createNewLayerWidget_();
    void destroyLastLayerWidget_();
    std::vector<LayerWidget*> layers_;
    QVBoxLayout * layerListLayout_;
};

#endif // LAYERSWIDGET_H
