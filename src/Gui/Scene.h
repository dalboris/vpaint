// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef SCENE_H
#define SCENE_H

#include <QObject>
#include <QMap>
#include <QList>
#include "TimeDef.h"
#include "Picking.h"
#include "ViewSettings.h"
#include "Background.h"

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

class Scene: public QObject
{
    Q_OBJECT
    
public:
    Scene();
    void copyFrom(Scene * other); // copy existing scene (for undo purpose)
    void clear(bool silent = false);
    ~Scene();

    QList<SceneObject*> sceneObjects() {return sceneObjects_;}

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

    // Scene Objects getters
    VectorAnimationComplex::VAC * vectorAnimationComplex();
    
    // GUI
    void populateToolBar(QToolBar * toolBar);

    // get relevant VAC to work on
    VectorAnimationComplex::VAC * getVAC_();

    // Other
    VectorAnimationComplex::InbetweenFace * createInbetweenFace();

    // Scene properties
    double left() const;
    double top() const;
    double width() const;
    double height() const;
    void setLeft(double x);
    void setTop(double y);
    void setWidth(double w);
    void setHeight(double h);
    void setCanvasDefaultValues();

    // Background. This address is guaranteed not to change.
    // This is different from vectorAnimationComplex(), which changes when
    // copyFrom is called.
    // XXX should vectorAnimationComplex() behave the same as background?
    const Background & background() const;
    Background & background();
    
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
    void addSceneObject(SceneObject * sceneObject, bool silent = false);
    QList<SceneObject*> sceneObjects_;

    int indexHovered_;

    double left_;
    double top_;
    double width_;
    double height_;

    Background background_;
};
    
#endif
