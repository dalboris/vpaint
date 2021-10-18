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

#ifndef LAYERSWIDGET_H
#define LAYERSWIDGET_H

#include <vector>

#include <QWidget>
#include "VAC/vpaint_global.h"

class QCheckBox;
class QLabel;
class QLineEdit;
class QVBoxLayout;
namespace VPaint
{
class Scene;
}
namespace impl_
{

/// One individual layer row in the Layers panel.
///
class Q_VPAINT_EXPORT LayerWidget: public QWidget
{
    Q_OBJECT

public:
    LayerWidget(int index);
    ~LayerWidget() override;

    int index() const;

    bool isActive() const;
    void setActive(bool b);

    bool visibility() const;
    void setVisibility(bool b);

    QString name() const;
    bool setName(const QString& name); // returns whether the name actually changed

    // Enter name editing.
    void startNameEditing();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

signals:
    void activated(int layerIndex);
    void visibilityChanged(int layerIndex);
    void nameChanged(int layerIndex);

    // This is called when name editing initiated from enterNameEditingMode()
    // is finished. It is not called when name editing initiated from double
    // click finished, or if name editing is aborted before being
    // finished.
    void nameEditingFinished(int layerIndex);

    // This signal is emitted when a user action is complete
    // and requires to add a copy of the scene to the undo stack
    void checkpoint();

private slots:
    void onVisibilityCheckBoxClicked_(bool);
    void onVisibilityCheckBoxStateChanged_(int);
    void onNameLineEditEditingFinished_();

private:
    int index_;    
    bool isActive_;
    QCheckBox * visibilityCheckBox_;
    QLabel * nameLabel_;
    QLineEdit * nameLineEdit_;

    enum class NameEditingReason_ {
        ExternalRequest,
        DoubleClick
    };
    void startNameEditing_(NameEditingReason_ reason);
    void abortNameEditing_();
    void finishNameEditing_();
    NameEditingReason_ nameEditingReason_;

    void updateBackground_();
};

} // namespace impl_

/// The whole Layers panel.
///
class Q_VPAINT_EXPORT LayersWidget: public QWidget
{
    Q_OBJECT

public:
    LayersWidget(VPaint::Scene * scene);
    ~LayersWidget() override;

    VPaint::Scene * scene() const;

private slots:
    void onLayerWidgetActivated_(int index);
    void onLayerWidgetVisibilityChanged_(int index);
    void onLayerWidgetNameChanged_(int index);
    void onLayerWidgetNameEditingFinished_(int index);
    void onLayerWidgetCheckpoint_();

    void onNewLayerClicked_();
    void onDeleteLayerClicked_();
    void onMoveLayerUpClicked_();
    void onMoveLayerDownClicked_();

    void onSceneLayerAttributesChanged_();

private:
    VPaint::Scene * scene_;
    void updateUiFromScene_();

    // Each LayerWidget is responsible for displaying info about a given layer.
    // When reordering the layers, the LayerWidget instances are not reordered,
    // but simply assigned a different layer to display info of.
    //
    // Note: index in layerWidgets_ is in reverse order as Scene::layer(int),
    // because in Scene (like in the *.vec file), layers are ordered from back
    // to front, while in the Layers panel, the background-most layer is at the
    // bottom.
    //
    // Invariants:
    // 1. numVisibleLayerWidgets_ == scene()->numLayers()
    // 2. numVisibleLayerWidgets_ <= layerWidgets_.size()
    //
    std::vector<impl_::LayerWidget*> layerWidgets_;
    int numVisibleLayerWidgets_;
    impl_::LayerWidget * activeLayerWidget_;

    void createNewLayerWidget_();
    QVBoxLayout * layerListLayout_;    
};

#endif // LAYERSWIDGET_H
