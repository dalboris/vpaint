// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef VAC_CELL_H
#define VAC_CELL_H

/*
 * Here is defined the base class for any Cell  of the VAC.
 *
 * Any cell:
 *   - has a pointer back to the VAC it belongs to
 *   - has an ID, which is unique w.r.t. its VAC
 *   - can be saved/load
 *   - can be drawn and selected: Spatial behaviour
 *   - has a lifetime: Temporal behaviour
 *
 * Any cell is either spatially (what the animator see):
 *   1) Vertex
 *   2) Edge
 *   3) Face
 *
 * And orthogonally any cell is either temporally:
 *   A) Key
 *   B) Inbetween
 *
 * Which lead to the six possible cells in our VAC:
 *   A1) KeyVertex
 *   A2) KeyEdge
 *   A3) KeyFace
 *   B1) InbetweenVertex
 *   B2) InbetweenEdge
 *   B3) InbetweenFace
 *
 * Because, for instance, all  instant cells share some common
 * behaviour, it  is very useful to  have a base  class for them
 * inheriting from Cell and  overiding some of the method, for
 * instant "bool  exists(time)" would  have been the  exact same
 * implementation for all the 3  cells KeyVertex, KeyEdge
 * and  KeyFace.  But because  this  is  also  true for  the
 * spatial  behaviours, its  leads to  the  unfamous inheritance
 * diamond pattern, e.g.:
 * 
 *                        _________
 *                       |   Cell  |
 *                        ---------
 *                       /          \ 
 *         ________________        _____________
 *        |     KeyCell    |      |  VertexCell |
 *         ----------------        -------------
 *                       \         /
 *                     _______________
 *                    |   KeyVertex   |
 *                     ---------------
 *
 * This is the reason why the abstract classes:
 *   - KeyCell
 *   - InbetweenCell
 *   - VertexCell
 *   - EdgeCell
 *   - FaceCell
 * inherit  only "virtually" the  abstract base  class "Cell",
 * so that the compiler takes  care not to duplicate the members
 * of Cell this way, leading to ambiguity:
 * 
 *            _________               _________
 *           |   Cell  |             |   Cell  |
 *            ---------               ---------
 *                |                       |
 *         ________________        _____________
 *        |     KeyCell    |      |  VertexCell |
 *         ----------------        -------------
 *                       \         /
 *                     _______________
 *                    |   KeyVertex   |
 *                     ---------------
 *
 * Tip: the  word "Cell"  in the name  of a class  reminds you
 *      that this class is  virtual, and hence that the pointers
 *      you manipulate can points to different kinds of cells.
 *      
 */

#include "../TimeDef.h"
#include "../ViewSettings.h"
#include "../View3DSettings.h"
#include "../CssStyle.h"
#include "CellList.h"
#include <QString>
#include <QRect>
#include <QColor>
class QTextStream;
class XmlStreamWriter;
class XmlStreamReader;

namespace VectorAnimationComplex
{

class CellObserver;
class KeyHalfedge;

// Convenient struct to store the bounding box of a cell
struct BBox
{
    // Members
    double minX, maxX, minY, maxY;

    // Convenient constructor
    BBox(double minX, double maxX, double minY, double maxY) :
        minX(minX), maxX(maxX), minY(minY), maxY(maxY) {}

    // Returns true if the two bounding boxes intersect
    bool intersects(const BBox & other) const;
};

// The abstract base class Cell
class Cell
{
public:
    // Constructor/Destructor are protected: it is the responsibility of VAC to create cells.

    // Destroy the cell. Same as vac()->deleteCell(this).
    void destroy();

    // Get the VAC this cell belongs to.
    VAC * vac() const { return vac_; }

    // Get the id of this cell.
    int id() const { return id_; }

    // Casting or Type Checking
    template <class CellClass> CellClass * to() { return dynamic_cast<CellClass*>(this); }
    Cell * toCell();
    KeyCell * toKeyCell();
    InbetweenCell * toInbetweenCell();
    VertexCell * toVertexCell();
    EdgeCell * toEdgeCell();
    FaceCell * toFaceCell();
    KeyVertex * toKeyVertex();
    KeyEdge * toKeyEdge();
    KeyFace * toKeyFace();
    InbetweenVertex * toInbetweenVertex();
    InbetweenEdge * toInbetweenEdge();
    InbetweenFace * toInbetweenFace();

    // Getting dimension
    int dimension();

    // Drawing and Picking, default implementation is:
    //   - drawing: call glColor(color), then drawRaw()

    //   - picking: call Picking::setID(), then drawRaw()
    //
    // If this behaviour is  enough (e.g., use only one color,
    // do  not use  some complex  picking), you  just  need to
    // reimplement drawRaw(), and  modify the protected member
    // "color".
    //
    // Note that it does take into account the selected and/or
    // highlighted state to choose the color to draw. Hence it
    // is better not to override other methods than drawRaw if
    // not necessary, to ensure homogeneous behaviour.
    // Todo: use the Visitor Pattern instead
    //       this would reduce the number of virtual functions
    //       and then the size of cell objects
    virtual void draw(Time time, ViewSettings & viewSettings);
    virtual void drawRaw(Time time, ViewSettings & viewSettings);
    void drawPick(Time time, ViewSettings & viewSettings);

    virtual void drawTopology(Time time, ViewSettings & viewSettings);
    virtual void drawRawTopology(Time time, ViewSettings & viewSettings);
    void drawPickTopology(Time time, ViewSettings & viewSettings);

    virtual void draw3D(View3DSettings & viewSettings);
    virtual void drawRaw3D(View3DSettings & viewSettings);
    virtual void drawPick3D(View3DSettings & viewSettings);

    // Check if the cell exists at this time
    virtual bool exists(Time /*time*/) const { return false; }
    virtual bool isBefore(Time /*time*/) const { return false; } // true iff cell lifespan \in (-infty,t)
    virtual bool isAfter(Time /*time*/) const { return false; } // true iff cell lifespan \in (t,+infty)
    virtual bool isAt(Time /*time*/) const { return false; } // true iff cell lifespan \in { t }. Cannot return true for inbetween cells

    // Highlighting and Selecting
    bool isHovered() const  { return isHovered_; }
    bool isSelected()    const  { return isSelected_;    }
    bool isHighlighted() const;

    // Topological Navigation Information
    // ------------ Boundary ------------
    CellSet boundary() const;
    virtual CellSet spatialBoundary() const;
    CellSet spatialBoundary(Time t) const;
    KeyCellSet temporalBoundary() const;
    virtual KeyCellSet beforeCells() const;
    virtual KeyCellSet afterCells() const;
    // -------------- Star --------------
    CellSet star() const;
    CellSet spatialStar() const;
    CellSet spatialStar(Time t) const;
    CellSet temporalStar() const;
    CellSet temporalStarBefore() const;
    CellSet temporalStarAfter() const;
    // ---------- Neighbourhood ---------
    CellSet neighbourhood() const;
    CellSet spatialNeighbourhood() const;
    CellSet spatialNeighbourhood(Time t) const;
    CellSet temporalNeighbourhood() const;
    CellSet temporalNeighbourhoodBefore() const;
    CellSet temporalNeighbourhoodAfter() const;

    // Safety check
    bool check() const;

    // Change the color of the cell
    QColor color() const;
    void setColor(const QColor & c);
    void setAutoColor();

    // Get the bounding box of this cell.
    BBox boundingBox() const;
    // Check if the bounding box of this cell intersects the bounding box of the other cell.
    bool boundingBoxIntersects(Cell * other) const;

    // Update cell boundary
    void updateBoundary(KeyVertex * oldVertex, KeyVertex * newVertex);
    void updateBoundary(const KeyHalfedge & oldHalfedge, const KeyHalfedge & newHalfedge);
    void updateBoundary(KeyEdge * oldEdge, const KeyEdgeList & newEdges);

    // Observers management
    void addObserver(CellObserver * observer);
    void removeObserver(CellObserver * observer);


protected:
    // --- Modifying star of boundary ---
    void addMeToStarOfBoundary_();
    void removeMeFromStarOfBoundary_();
    void removeMeFromStarOf_(Cell * c);
    // --- Modifying spatial star of boundary ---
    void addMeToSpatialStarOf_(Cell * c);
    void removeMeFromSpatialStarOf_(Cell * c);
    // --- Modifying temporal star of boundary ---
    void addMeToTemporalStarBeforeOf_(Cell *c);
    void addMeToTemporalStarAfterOf_(Cell *c);
    void removeMeFromTemporalStarBeforeOf_(Cell *c);
    void removeMeFromTemporalStarAfterOf_(Cell *c);

    CellSet geometryDependentCells_();
    virtual BBox computeBoundingBox_() const = 0;
    void geometryChanged_(); // derived classes must call this when their geometry changes
    virtual void clearCachedGeometry_();

private:
    // Embedding in VAC
    friend class VAC;
    VAC * vac_;
    int id_;

    // Trusting operators
    friend class Operator;
    virtual bool check_() const=0;

    // highlighting and selecting
    bool isHovered_;
    bool isSelected_;
    void setHovered(bool b) { isHovered_ = b;    }
    void setSelected(bool b)    { isSelected_    = b;    }

    // Observer management
    QSet<CellObserver*> observers_;

    // Non-Virtual Interface idiom
    bool isPickable(Time time) const;
    virtual bool isPickableCustom(Time time) const;
    virtual void drawPickCustom(Time time, ViewSettings & viewSettings);
    virtual void drawPickTopologyCustom(Time time, ViewSettings & viewSettings);

    // Update boundary
    void updateBoundary_preprocess();
    void updateBoundary_postprocess();
    virtual void updateBoundary_impl(KeyVertex * oldVertex, KeyVertex * newVertex);
    virtual void updateBoundary_impl(const KeyHalfedge & oldHalfedge, const KeyHalfedge & newHalfedge);
    virtual void updateBoundary_impl(KeyEdge * oldEdge, const KeyEdgeList & newEdges);

    // Star: Those are the boundary's back-pointers needed for efficiency
    //       (Otherwise, if implemented as a method, it would be necessary
    //        to visit all the cells in the VAC to check those whose boundary
    //        contains this cell)
    CellSet spatialStar_;
    CellSet temporalStarBefore_; // We know they are animated cells, but not enforced to be consistent
    CellSet temporalStarAfter_;  // with spatial star (in which case we know they are either edges of faces)
                                 // This emphasizes the idea that we do not store any semantics for the star,
                                 // only for the boundary, and that the star is only stored to inform all of them
                                 // consistently when a change happened to the boundary

    // Bounding box
    mutable BBox boundingBox_;
    mutable bool boundingBoxIsDirty_;

protected:
    // Make a call to  glColor, taking into account the member
    // color,  the selected state  and the  highlighted state.
    // Made virtual  for flexibility (automatically  called by
    // draw() if not overriden), but you should avoid override
    // it to ensure homogeneous behaviour.
    virtual QColor getColor(Time time, ViewSettings & viewSettings) const;
    virtual void glColor_(Time time, ViewSettings & viewSettings);
    virtual void glColorTopology_();
    virtual void glColor3D_();
    double colorHighlighted_[4];
    double colorSelected_[4];
    double color_[4];

    // Styling
    CssStyle style_;

    // Get the cell with ID id that belongs to the same VAC as this cell. Same as vac()->getCell(id).
    Cell * getCell(int id);

    // Protected constructor, so only VAC and derived classes can call it.
    // It creates a cell with VAC `vac`. `vac` must be non null.
    // It is not inserted in the VAC's containers. No valid ID is given.
    // All the above must be done by the VAC right after its creation.
    Cell(VAC * vac);

    // Virtual protected destructor and helper methods for destruction.
    // Note: These helper methods *must* be called before and outside
    //       the actual Cell destructor since they rely on the virtual
    //       method boundary().
    virtual ~Cell()=0;
    void destroyStar();
    void informBoundaryImGettingDestroyed();


    // Splitting
    /*
    void split_callBoundaryChanged(Cell * cell, const SplitMap & splitMap);
    virtual void split_boundaryChanged(const SplitMap & splitMap);
    */
    // only need to be  inherited by potential star cells of
    // cells  that can  be splitted.  Their dimension  is at
    // least 2, and hence only includes:
    //   - Key Face
    //   - Animated Edge
    //   - Animated Face (TODO)
    //

// --------- Cloning, Assigning, Copying, Serializing ----------


protected:
    // Cloning (caution: it is the caller's responsibility to
    //                   insert it in the appropriate vac)
    Cell(Cell * other);
    virtual Cell * clone()=0;
    virtual void remapPointers(VAC * newVAC);

    // Serializating
    void write(XmlStreamWriter & xml) const;
    virtual QString xmlType_() const;
    virtual void write_(XmlStreamWriter & xml) const;

    // Unserializing
    Cell(VAC * vac, XmlStreamReader & xml);
    virtual void read2ndPass();

    // Serializing / Unserializing (DEPRECATED)
    void save(QTextStream & out);
    virtual void save_(QTextStream & out); // called at the end of save(), for derived object
    virtual QString stringType() const {return "Cell";}
    Cell(VAC * vac, QTextStream & in);
    static Cell * read1stPass(VAC * vac, QTextStream & in);

    // Export
    virtual void exportSVG(Time t, QTextStream & out);
};
        
    
}

#endif
