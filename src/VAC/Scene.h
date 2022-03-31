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

#ifndef SCENE_H
#define SCENE_H

#include <QObject>
#include <QMap>
#include <QList>
#include "TimeDef.h"
#include "Picking.h"
#include "ViewSettings.h"
#include "VAC/vpaint_global.h"

class Background;
class View;
struct MouseEvent;
class QKeyEvent;
class SceneObject;
class QTextStream;
class XmlStreamWriter;
class XmlStreamReader;
class QToolBar;
namespace VectorAnimationComplex
{
class VAC;
class InbetweenFace;
}
class QDir;
class Layer;

namespace VPaint {
class Q_VPAINT_EXPORT Scene: public QObject
{
    Q_OBJECT
    
public:
    // Creates an empty scene, that is, a scene with zero layers
    Scene();

    // Creates a scene with one default layer
    static Scene * createDefaultScene();

    // Makes this scene a clone of the other scene
    // Note: "copy" would be a better name but is already taken (copy/paste slots)
    void copyFrom(Scene * other);

    // Clears the scene by deleting all its layers. If silent is true, then
    // this function no signal will be emitted.
    void clear(bool silent = false);

    // Destroys the scene
    ~Scene();

    // Keyboard events
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

    // Drawing (assumes a 2D OpenGL context is setup)
    void drawCanvas(ViewSettings & viewSettings);
    void draw(Time time, ViewSettings & viewSettings);
    void drawPick(Time time, ViewSettings & viewSettings);

    // XXX todo: there should be draw3D here too (not only in VAC),
    //           responsible for instance to draw the canvas

    // Highlighting and Selecting
    void setHoveredObject(Time time, int index, int id);
    void setNoHoveredObject();
    void select(Time time, int index, int id);
    void deselect(Time time, int index, int id);
    void toggle(Time time, int index, int id);
    void deselectAll(Time time);

    // Emit signals
    void emitChanged() {emit changed();}
    void emitCheckpoint() {emit checkpoint();}

    // Save and load
    void exportSVG(Time t, QTextStream & out);
    void save(QTextStream & out);
    void read(QTextStream & in);
    void writeAllLayers(XmlStreamWriter & xml);
    void readOneLayer(XmlStreamReader & xml);
    void readCanvas(XmlStreamReader & xml);
    void writeCanvas(XmlStreamWriter & xml);
    void relativeRemap(const QDir & oldDir, const QDir & newDir);

    // Get layer from index
    int numLayers() const;
    Layer* layer(int i) const;

    // Active layer
    void setActiveLayer(int i);
    Layer * activeLayer() const;
    int activeLayerIndex() const; // returns -1 if no active layers.
    VectorAnimationComplex::VAC * activeVAC() const;
    Background * activeBackground() const;

    // Create layer above active layer, or last if no active layer
    // Set as active layer
    Layer * createLayer();
    Layer * createLayer(const QString & name);
    void addLayer(Layer * layer );
    void moveActiveLayerUp();
    void moveActiveLayerDown();

    // destroy the given layer
    void destroyActiveLayer();
    
    // GUI
    void populateToolBar(QToolBar * toolBar);

    // Other
    VectorAnimationComplex::InbetweenFace * createInbetweenFace();

    // Scene properties
    // XXX refactor this out in a Canvas class
    double left() const;
    double top() const;
    double width() const;
    double height() const;
    void setLeft(double x);
    void setTop(double y);
    void setWidth(double w);
    void setHeight(double h);
    void setCanvasDefaultValues();
    
public slots:
    // --------- Tools ----------
    void test();
    void deleteSelectedCells();
    void smartDelete();
    void cut(VectorAnimationComplex::VAC* & clipboard);
    void copy(VectorAnimationComplex::VAC* & clipboard);
    void paste(VectorAnimationComplex::VAC* & clipboard);
    void createFace();
    void addCyclesToFace();
    void removeCyclesFromFace();
    void changeColor();
    void raise();
    void lower();
    void raiseToTop();
    void lowerToBottom();
    void altRaise();
    void altLower();
    void altRaiseToTop();
    void altLowerToBottom();
    void changeEdgeWidth();
    void glue();
    void unglue();
    void uncut();

    // ----- animation -----
    void inbetweenSelection();
    void keyframeSelection();
    void motionPaste(VectorAnimationComplex::VAC* & clipboard);

    // ----- others -----
    void resetCellsToConsiderForCutting();
    void updateCellsToConsiderForCutting();

    // ------ Selection ----
    void selectAllInFrame();
    void selectAllInAnimation();
    void deselectAll();
    void invertSelection();
    void selectConnected();
    void selectClosure();
    void selectVertices();
    void selectEdges();
    void selectFaces();
    void deselectVertices();
    void deselectEdges();
    void deselectFaces();
    void selectKeyCells();
    void selectInbetweenCells();
    void deselectKeyCells();
    void deselectInbetweenCells();
    void selectKeyVertices();
    void selectKeyEdges();
    void selectKeyFaces();
    void deselectKeyVertices();
    void deselectKeyEdges();
    void deselectKeyFaces();
    void selectInbetweenVertices();
    void selectInbetweenEdges();
    void selectInbetweenFaces();
    void deselectInbetweenVertices();
    void deselectInbetweenEdges();
    void deselectInbetweenFaces();

    
signals:
    void changed();    // can be emitted for any reason, even a change of color.
                       // if called several times in a row, the view will still
                       // be redrawn only once.

    void checkpoint(); // when emitted, the whole scene is copied and put in the
                       // undo list. Make sure to call this only once, when necessary

    void needUpdatePicking(); // Make sure to call this only once, when necessary

    void selectionChanged();
    void layerAttributesChanged();

    
private:
    // Layers are ordered back to front,
    // Example: layers_[0] is the bottom-most layer, rendered first
    void addLayer_(Layer * layer, bool silent = false);
    QList<Layer*> layers_;
    int activeLayerIndex_;

    int indexHovered_;

    double left_;
    double top_;
    double width_;
    double height_;
};
}
    
#endif
