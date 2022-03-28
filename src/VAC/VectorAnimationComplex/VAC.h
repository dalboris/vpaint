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

#ifndef VAC_VAC_H
#define VAC_VAC_H

#include <QSet>
#include <QMap>
#include <QColor>

#include "../SceneObject.h"

#include "ForwardDeclaration.h"
#include "CellList.h"
#include "Cell.h"
#include "ZOrderedCells.h"
#include "Eigen.h"
#include "TransformTool.h"
#include "EdgeSample.h"

#include "../View3DSettings.h"
#include "VAC/vpaint_global.h"
namespace VPaint
{
class Scene;
}
class XmlStreamWriter;
class XmlStreamReader;

namespace VectorAnimationComplex
{

class EdgeGeometry;
class LinearSpline;

class Operator;
class ProperCycle;
class Cycle;
class AnimatedCycle;
class Path;
class AnimatedVertex;
class KeyHalfedge;
class PreviewKeyFace;
class BoundingBox;

class Q_VPAINT_EXPORT VAC: public SceneObject
{
    Q_OBJECT

public:
    // Constructors and Destructor
    VAC();
    VAC(QTextStream & in);
    ~VAC();
    VAC * clone();
    QString stringType();
    void clear();

    // Serialization / Unserialization
    void write(XmlStreamWriter & xml);
    void read(XmlStreamReader & xml);

    // Initializations
    void initNonCopyable();
    void initCopyable();

    // VAC extraction and insertion
    QMap<int, int> import(VAC * other, bool selectImportedCells = false, bool isMousePaste = false); // insert a copy of other inside this
    VAC * subcomplex(const CellSet & subcomplexCells); // Create a new VAC whose cells are cells

    // Drawing
    void draw(Time time, ViewSettings & viewSettings);
    void drawPick(Time time, ViewSettings & viewSettings);

    // Selecting and Highlighting
    void setHoveredObject(Time time, int id);
    void setNoHoveredObject();
    void select(Time time, int id);
    void deselect(Time time, int id);
    void toggle(Time time, int id);
    void deselectAll(Time time);
    void deselectAll();
    void invertSelection();

    // Get higlighted and selected state
    Cell * hoveredCell() const;
    const CellSet & selectedCells() const;
    const CellSet& hoveredCells() const;
    int numSelectedCells() const;

    // Get hovered transform widget id
    int hoveredTransformWidgetId() const;

    // Modify highligthed and seleted state
    void setHoveredCell(Cell * cell);
    void hoveveredConnected(bool emitSignal = false);
    void setNoHoveredCell();
    void addToHovered(Cell * cell, bool emitSignal = false);
    void addToHovered(const CellSet & cells, bool emitSignal = false);
    void setNoHoveredAllCells();
    void addToSelection(Cell * cell, bool emitSignal = true);
    void addToSelection(const CellSet & cells, bool emitSignal = true);
    void setSelectedCell(Cell * cell, bool emitSignal = true);
    void setSelectedCells(const CellSet & cells, bool emitSignal = true);
    void removeFromSelection(Cell * cell, bool emitSignal = true);
    void removeFromSelection(const CellSet & cells, bool emitSignal = true);
    void toggleSelection(Cell * cell, bool emitSignal = true);
    void toggleSelection(const CellSet & cells, bool emitSignal = true);
    void informTimelineOfSelection();

    // Get element by ID: returns NULL if invalid ID or type.
    Cell * getCell(int id);
    KeyVertex * getKeyVertex(int id);
    KeyEdge * getKeyEdge(int id);
    KeyFace * getKeyFace(int id);
    InbetweenVertex * getInbetweenVertex(int id);
    InbetweenEdge * getInbetweenEdge(int id);
    InbetweenFace * getInbetweenFace(int id);

    // Get all cells of a given type
    CellSet cells();
    VertexCellList vertices();
    EdgeCellList edges();
    FaceCellList faces();
    KeyEdgeList instantEdges();
    KeyVertexList instantVertices();

    // Get all cells of a given type existing at a given time
    CellSet cells(Time time);
    EdgeCellList edges(Time time);
    KeyEdgeList instantEdges(Time time);
    KeyVertexList instantVertices(Time time);

    // Get all cells, ordered
    const ZOrderedCells & zOrdering() const;

    // Populate MainWindow toolbar (called once, when launching application)
    static void populateToolBar(QToolBar * toolBar, VPaint::Scene * scene);

    /////////////////////////////////////////////////////////////////
    //                 MOUSE PMR ACTIONS                           //

    // -- Drag and drop --
    void prepareDragAndDrop(double x0, double y0, Time time);
    void performDragAndDrop(double x, double y);
    void completeDragAndDrop();

    // -- Transform selection --
    void beginTransformSelection(double x0, double y0, Time time);
    void continueTransformSelection(double x, double y);
    void endTransformSelection();

    // -- Temporal Drag and drop --
    void prepareTemporalDragAndDrop(Time t0);
    void performTemporalDragAndDrop(Time t);
    void completeTemporalDragAndDrop();

    // -- Rectangle of selection --
    void beginRectangleOfSelection(double x, double y, Time time);
    void continueRectangleOfSelection(double x, double y);
    void setSelectedCellsFromRectangleOfSelection();
    void setSelectedCellsFromRectangleOfSelection(Qt::KeyboardModifiers modifiers);
    void endRectangleOfSelection();

    // -- Sketch --
    void beginSketchEdge(double x, double y, double w, Time time);
    void continueSketchEdge(double x, double y, double w);
    void endSketchEdge();

    // -- Sculpt --
    void updateSculpt(double x, double y, Time time);
    // Deform
    void beginSculptDeform(double x, double y);
    void continueSculptDeform(double x, double y);
    void endSculptDeform();
    // Change edge width
    void beginSculptEdgeWidth(double x, double y);
    void continueSculptEdgeWidth(double x, double y);
    void endSculptEdgeWidth();
    // Smooth
    void beginSculptSmooth(double x, double y);
    void continueSculptSmooth(double x, double y);
    void endSculptSmooth();

    // -- Cut Face --
    void beginCutFace(double x, double y, double w, KeyVertex * startVertex);
    void continueCutFace(double x, double y, double w);
    void endCutFace(KeyVertex * endVertex);

    //Colors change
    void changeEdgesColor();
    void changeFacesColor();
    void adjusSelectedAndHighlighted(Cell* cell);
    void adjustSelectColors(Cell* cell);
    void adjustSelectColorsAll();

    /////////////////////////////////////////////////////////////////
    //                 MOUSE CLIC ACTIONS                          //

    // -- Cut edge --
    // This is actually more generic: if no edge is highlighted, it still creates a vertex
    // It is more like a cutAt(x,y);
    KeyVertex * split(double x, double y, Time time, bool interactive = true);

    // -- Paint Bucket tool --
    // paint() returns the painted cell, if any. Might be an existing
    // cell that has been re-colored, or a new face that has been created
    void updateToBePaintedFace(double x, double y, Time time);
    Cell * paint(double x, double y, Time time);

    /////////////////////////////////////////////////////////////////
    //         SELECTION (MOUSE CLIC ACTIONS)                      //

public slots:
    void selectAll(bool emitSignal = true);
    void selectAllAtTime(Time time, bool emitSignal = true);
    void selectConnected(bool emitSignal = true);
    void selectClosure(bool emitSignal = true);
    void selectVertices(bool emitSignal = true);
    void selectEdges(bool emitSignal = true);
    void selectFaces(bool emitSignal = true);
    void deselectVertices(bool emitSignal = true);
    void deselectEdges(bool emitSignal = true);
    void deselectFaces(bool emitSignal = true);
    void selectKeyCells(bool emitSignal = true);
    void selectInbetweenCells(bool emitSignal = true);
    void deselectKeyCells(bool emitSignal = true);
    void deselectInbetweenCells(bool emitSignal = true);
    void selectKeyVertices(bool emitSignal = true);
    void selectKeyEdges(bool emitSignal = true);
    void selectKeyFaces(bool emitSignal = true);
    void deselectKeyVertices(bool emitSignal = true);
    void deselectKeyEdges(bool emitSignal = true);
    void deselectKeyFaces(bool emitSignal = true);
    void selectInbetweenVertices(bool emitSignal = true);
    void selectInbetweenEdges(bool emitSignal = true);
    void selectInbetweenFaces(bool emitSignal = true);
    void deselectInbetweenVertices(bool emitSignal = true);
    void deselectInbetweenEdges(bool emitSignal = true);
    void deselectInbetweenFaces(bool emitSignal = true);


    /////////////////////////////////////////////////////////////////
    //     ONE-SHOT KEYBOARD OR INSTANT TOOL ACTIONS               //

public slots:
    void test();
    void deleteSelectedCells();
    void smartDelete();
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
    void cut(VAC* & clipboard);
    void copy(VAC* & clipboard);
    void paste(VAC* & clipboard, bool isMousePaste = false);
    void resetCellsToConsiderForCutting();
    void updateCellsToConsiderForCutting();
    // -- animation --
    void inbetweenSelection();
    void keyframeSelection();
    void motionPaste(VAC* & clipboard);

public:
    // Safely create cells given a valid boundary.
    // These methods allocate the new cell and insert it in the VAC
    // todo: move in private. // <- obsolete comment?
    //       the only issue now is that InbetweenEdge uses newInbetweenVertex() to
    //       automagically connect provided temporal boundary. But this has to be changed:
    //       it is the responsibility of VAC to create the appropriate complete boundary
    //       of any cell, and to give it to the constructor of the cell. Helpers boundary can
    //       include algorithms if needed, but cells constructor must have valid boundary already
    //       specified.
    KeyVertex * newKeyVertex(Time time, const Eigen::Vector2d & pos = Eigen::Vector2d(0,0));
    KeyVertex * newKeyVertex(Time time, const EdgeSample& sample);
    KeyEdge * newKeyEdge(Time time,
               KeyVertex * left,
               KeyVertex * right,
               EdgeGeometry * geometry = 0, double width = 0); // if geometry = 0, create a straight line
    KeyEdge * newKeyEdge(Time time,
               EdgeGeometry * geometry = 0);
    InbetweenVertex * newInbetweenVertex(
               KeyVertex * before = 0,
               KeyVertex * after = 0);
    InbetweenEdge * newInbetweenEdge(
            const Path & beforePath,
            const Path & afterPath,
            const AnimatedVertex & startAnimatedVertex,
            const AnimatedVertex & endAnimatedVertex);
    InbetweenEdge * newInbetweenEdge(
            const Cycle & beforeCycle,
            const Cycle & afterCycle);
    KeyFace * newKeyFace(const Time & t);
    KeyFace * newKeyFace(const Cycle & cycle);
    KeyFace * newKeyFace(const QList<Cycle> & cycles);
    InbetweenFace * newInbetweenFace(const QList<AnimatedCycle> & cycles,
                                     const QSet<KeyFace*> & beforeFaces,
                                     const QSet<KeyFace*> & afterFaces);


    // safely delete objects
    void deleteCells(const QSet<int>  & cellIds);
    void deleteCells(const CellSet & cells);
    void deleteCell(Cell * cell);
    void smartDeleteCell(Cell * cell);
    bool atomicSimplifyAtCell(Cell * cell);
    bool simplifyAtCell(Cell * cell);

    // Check the invariants of the VAC
    bool check() const;
    bool checkContains(const Cell * c) const;


protected:
    // Save & Load
    void save_(QTextStream & out);
    virtual void exportSVG_(Time t, QTextStream & out);
    void read2ndPass_();

signals:
    void selectionChanged();

private:
    // Trusting operators
    friend class Operator;

    // All cells in vac, accessible by ID
    QMap<int, Cell*> cells_;
    void removeCell_(Cell * cell);
    void insertCell_(Cell * cell);
    void insertCellLast_(Cell * cell);

    // Managing IDs
    int getAvailableID();
    void deleteAllCells();
    void setMaxID_(int maxID);
    int maxID_;

    // User interactivity
    Time timeInteractivity_;

    // Rectangle of selection
    double rectangleOfSelectionStartX_;
    double rectangleOfSelectionStartY_;
    double rectangleOfSelectionEndX_;
    double rectangleOfSelectionEndY_;
    bool drawRectangleOfSelection_;
    CellSet rectangleOfSelectionSelectedBefore_;
    CellSet cellsInRectangleOfSelection_;

    // Drawing a new stroke
    void insertSketchedEdgeInVAC();
    void insertSketchedEdgeInVAC(double tolerance, bool useFaceToConsiderForCutting = true);
    void drawSketchedEdge(Time time, ViewSettings & viewSettings) const;
    void drawTopologySketchedEdge(Time time, ViewSettings & viewSettings) const;
    LinearSpline * sketchedEdge_;
    double ds_;
    KeyFace * hoveredFaceOnMousePress_;
    KeyFace * hoveredFaceOnMouseRelease_;
    KeyFaceSet hoveredFacesOnMouseMove_;
    KeyFaceSet facesToConsiderForCutting_;
    KeyEdgeSet edgesToConsiderForCutting_;

    // Create a face, or inserting/removing cycles
    QList<Cycle> createFace_computeCycles();

    // Splitting
    struct SplitInfo
    {
        KeyEdge * oldEdge;
        KeyEdgeList newEdges;
        KeyVertexList newVertices;
    };
    KeyVertex * cutFaceAtVertex_(KeyFace * face, double x, double y);
    KeyVertex * cutEdgeAtVertex_(KeyEdge * edge, double s);
    SplitInfo cutEdgeAtVertices_(KeyEdge * edgeToSplit, const std::vector<double> & splitValues);

    // Gluing
    void glue_(KeyVertex * v1, KeyVertex * v2);
    void glue_(KeyEdge * e1, KeyEdge * e2);
    void glue_(const KeyHalfedge & h1, const KeyHalfedge & h2);

    // Ungluing
    int nUses_(KeyVertex * v);
    int nUses_(KeyEdge * e);
    void unglue_(KeyVertex * v);
    void unglue_(KeyEdge * e);

    // Uncutting
    bool uncut_(KeyVertex * v);
    bool uncut_(KeyEdge * e);

    // Smart deleting
    void smartDelete_(const CellSet & cellsToDelete);

    // Cut a face along an edge
    KeyVertex * cut_startVertex_;
    struct CutFaceFeedback
    {
        KeyFaceSet newFaces;
        KeyFaceSet deletedFaces;
    };
    bool cutFace_(KeyFace * f, KeyEdge * edge, CutFaceFeedback * feedback = 0);

    // Inbetweening
    InbetweenVertex * inbetweenVertices_(KeyVertex * v1, KeyVertex * v2);
    InbetweenEdge * inbetweenEdges_(KeyEdge * e1, KeyEdge * e2);

    // Keyframing
    KeyCellSet keyframe_(const CellSet & cells, Time time);
    KeyVertex * keyframe_(InbetweenVertex * svertex, Time time);
    KeyEdge * keyframe_(InbetweenEdge * sedge, Time time);
    KeyFace * keyframe_(InbetweenFace * sface, Time time);

    // Sculpting
    KeyEdge * sculptedEdge_;

    // Drag and drop
    KeyVertexSet draggedVertices_;
    KeyEdgeSet draggedEdges_;
    double x0_, y0_;

    // Temporal drag and drop
    KeyCellSet draggedKeyCells_;
    QMap<KeyCell*, Time> draggedKeyCellTime_;
    Time t0_;
    Time deltaTMin_;
    Time deltaTMax_;

    // Painting
    PreviewKeyFace * toBePaintedFace_;

    // Cut-Copy-Paste
    Time timeCopy_;

    // Selecting and highlighting
    int hoveredTransformWidgetId_;
    Cell * hoveredCell_;
    CellSet selectedCells_;
    CellSet hoveredCells_;

    // Z-layering
    ZOrderedCells zOrdering_;

    // Smart aggregation of signals
    void emitSelectionChanged_();
    void beginAggregateSignals_();
    void endAggregateSignals_();
    int signalCounter_;
    bool shouldEmitSelectionChanged_;

    // Transform tool
    TransformTool transformTool_;
    friend class TransformTool;

    double pasteDeltaX_;
    double pasteDeltaY_;
};

}

#endif
