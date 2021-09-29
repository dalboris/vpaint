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

#ifndef VAC_CELL_H
#define VAC_CELL_H

// Remove C4250 warning in MSVC ('class1' : inherits 'class2::member' via dominance)
#if _MSC_VER && !__INTEL_COMPILER
#pragma warning(disable:4250)
#endif

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
#include "CellList.h"
#include "Triangles.h"
#include "BoundingBox.h"
#include <QString>
#include <QRect>
#include <QColor>
#include "VAC/vpaint_global.h"

class QTextStream;
class XmlStreamWriter;
class XmlStreamReader;

namespace VectorAnimationComplex
{

class CellObserver;
class KeyHalfedge;

// The abstract base class Cell
class Q_VPAINT_EXPORT Cell
{

//###################################################################
//                            CORE
//###################################################################

public:
    // Get the VAC this cell belongs to, and the cell's id
    VAC * vac() const { return vac_; }
    int id() const { return id_; }

    // Destroy the cell safely (syntactic sugar for vac()->deleteCell(this))
    void destroy();

    // Observers
    void addObserver(CellObserver * observer);
    void removeObserver(CellObserver * observer);

protected:
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

    // Cloning (caution: it is the caller's responsibility to
    //                   insert it in the appropriate vac)
    Cell(Cell * other);
    virtual Cell * clone()=0;
    virtual void remapPointers(VAC * newVAC);

    // Get cell from id (syntactic sugar for vac()->getCell(id))
    Cell * getCell(int id);

private:
    // Embedding in VAC
    friend class VAC;
    VAC * vac_;
    int id_;

    // Observers
    QSet<CellObserver*> observers_;


//###################################################################
//                          TYPE CASTING
//###################################################################

public:
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


//###################################################################
//                           TIME
//###################################################################

public:
    // Check if the cell exists at this time
    virtual bool exists(Time /*time*/) const { return false; }
    virtual bool isBefore(Time /*time*/) const { return false; } // true iff cell lifespan \in (-inf,t)
    virtual bool isAfter(Time /*time*/) const { return false; } // true iff cell lifespan \in (t,+inf)
    virtual bool isAt(Time /*time*/) const { return false; } // true iff cell lifespan \in { t }. Cannot return true for inbetween cells


//###################################################################
//                          TOPOLOGY
//###################################################################

public:
    // Get cell dimension
    int dimension();

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

    // Update cell boundary as a result of a split
    void updateBoundary(KeyVertex * oldVertex, KeyVertex * newVertex);
    void updateBoundary(const KeyHalfedge & oldHalfedge, const KeyHalfedge & newHalfedge);
    void updateBoundary(KeyEdge * oldEdge, const KeyEdgeList & newEdges);

    // Safety check
    bool check() const;

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

private:
    // Trusting operators
    friend class Operator;
    virtual bool check_() const=0;

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


//###################################################################
//                 HIGHLIGHTING / SELECTING / DRAWING
//###################################################################

public:
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

    // Highlighting and Selecting
    bool isHovered() const  { return isHovered_; }
    bool isSelected()    const  { return isSelected_;    }
    bool isHighlighted() const;

    // Change the color of the cell
    QColor color() const;
    void setColor(const QColor & c);

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

private:
    // highlighting and selecting
    bool isHovered_;
    bool isSelected_;
    void setHovered(bool b) { isHovered_ = b;    }
    void setSelected(bool b) { isSelected_    = b;    }

    // Non-Virtual Interface idiom
    bool isPickable(Time time) const;
    virtual bool isPickableCustom(Time time) const;
    virtual void drawPickCustom(Time time, ViewSettings & viewSettings);
    virtual void drawPickTopologyCustom(Time time, ViewSettings & viewSettings);


//###################################################################
//                            I/O
//###################################################################

protected:
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


//###################################################################
//                         GEOMETRY
//###################################################################

public:
    // Get all the triangles to be rendered at given time
    const Triangles & triangles(Time t) const;

    // Get the bounding box of this cell at time t
    const BoundingBox & boundingBox(Time t) const;
    const BoundingBox & outlineBoundingBox(Time t) const;

    // Get the bounding box of this cell for all time t
    virtual BoundingBox boundingBox() const=0;
    virtual BoundingBox outlineBoundingBox() const=0;

    // Cell-BoundingBox intersection test. It uses the actual geometry of the
    // cell, i.e. it is more expensive but more accurate than:
    //     boundingBox(t).intersects(bb);
    bool intersects(Time t, const BoundingBox & bb) const;

protected:
    // Method to be called by derived classes when their geometry changes
    void processGeometryChanged_();

    // Clear cached geometry (derived classes caching more data may specialize it)
    virtual void clearCachedGeometry_();

private:
    // Cached triangulations and bounding boxes (the integer represent a 1/60th of frame)
    mutable QMap<int,Triangles> triangles_;
    mutable QMap<int,BoundingBox> boundingBoxes_;
    mutable QMap<int,BoundingBox> outlineBoundingBoxes_;

    // Compute triangulation for time t (must be implemented by derived classes)
    virtual void triangulate_(Time t, Triangles & out) const=0;

    // Compute outline bounding box for time t (must be implemented by derived classes)
    virtual void computeOutlineBoundingBox_(Time t, BoundingBox & out) const=0;

    // Return the list of cells whose geometry depends on this cell's geometry
    CellSet geometryDependentCells_();
};
    
}

#endif
