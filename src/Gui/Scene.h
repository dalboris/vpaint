// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef SCENE_H
#define SCENE_H

#include <QObject>
#include <QMap>
#include <QList>
#include "TimeDef.h"
#include "Picking.h"
#include "ViewSettings.h"

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

class Scene: public QObject
{
    Q_OBJECT
    
public:
    Scene();
    void copyFrom(Scene * other);
    void clear(bool silent = false);
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
    void write(XmlStreamWriter & xml);
    void read(XmlStreamReader & xml);
    void readCanvas(XmlStreamReader & xml);
    void writeCanvas(XmlStreamWriter & xml);
    void relativeRemap(const QDir & oldDir, const QDir & newDir);

    // Get the active layer
    VectorAnimationComplex::VAC * activeLayer();
    
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

    // Background
    Background * background() const;
    
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
    void selectAll();
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

    
signals:
    void changed();    // can be emitted for any reason, even a change of color.
                       // if called several times in a row, the view will still
                       // be redrawn only once.

    void checkpoint(); // when emitted, the whole scene is copied and put in the
                       // undo list. Make sure to call this only once, when necessary

    void needUpdatePicking(); // Make sure to call this only once, when necessary

    void selectionChanged();

    
private:
    void addLayer(SceneObject * layer, bool silent = false);
    QList<SceneObject*> layers_;

    int indexHovered_;

    double left_;
    double top_;
    double width_;
    double height_;

    Background * background_;
};
    
#endif
