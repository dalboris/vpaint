// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef VAC_ANIMATED_EDGE_H
#define VAC_ANIMATED_EDGE_H

#include "InbetweenCell.h"
#include "EdgeCell.h"

#include "Path.h"
#include "Cycle.h"
#include "AnimatedVertex.h"
#include "EdgeSample.h"

#include <QList>
#include <QPair>

namespace VectorAnimationComplex
{

class InbetweenEdge: public InbetweenCell, public EdgeCell
{
public:
    // Construct a inbetween open edge
    InbetweenEdge(VAC * vac,
                  const Path & beforePath,
                  const Path & afterPath,
                  const AnimatedVertex & startAnimatedVertex,
                  const AnimatedVertex & endAnimatedVertex);

    // Construct a inbetween closed edge
    InbetweenEdge(VAC * vac,
                  const Cycle & beforeCycle,
                  const Cycle & afterCycle);

    // Check if closed or open
    bool isClosed() const;

    // Setters
    void setBeforeCycleStartingPoint(double s0);
    void setAfterCycleStartingPoint(double s0);

    // Getters
    double beforeCycleStartingPoint() const;
    double afterCycleStartingPoint() const;

    // Get boundary cells
    KeyCellSet beforeCells() const;
    KeyCellSet afterCells() const;
    VertexCellSet startVertices() const;
    VertexCellSet endVertices() const;

    // Get semantic boundary
    //  -> inbetween open edge case
    Path beforePath() const;
    Path afterPath() const;
    AnimatedVertex startAnimatedVertex() const;
    AnimatedVertex endAnimatedVertex() const;
    //   -> inbetween closed edge case
    Cycle beforeCycle() const;
    Cycle afterCycle() const;

    // Convenient getters
    VertexCell * startVertex(Time time) const;
    VertexCell * endVertex(Time time) const;

    // Drawing
    void glColor3D_();
    void drawRaw3D(View3DSettings & viewSettings);
    void triangulate(Time time, Triangles & out);
    void triangulate(double width, Time time, Triangles & out);
    //void drawRaw(Time time);
    //void drawRawTopology(Time time, ViewSettings & viewSettings);
    //void resetSampling();

    // Other
    QList<EdgeSample> getSampling(Time time) const; // Note: repeat start and end vertices even when closed.
    QList<Eigen::Vector2d> getGeometry(Time time); // Note: repeat start and end vertices even when closed.

private:
    // Cached geometry
    QList< QList<Eigen::Vector3d> > surf_;
    QList< QList<Eigen::Vector3d> > norm_;
    virtual void clearCachedGeometry_();
    void computeInbetweenSurface(View3DSettings & viewSettings);

    // Trusting operators
    friend class VAC;
    friend class Operator;
    bool check_() const;

    ~InbetweenEdge();

    // Update Boundary
    void updateBoundary_impl(KeyVertex * oldVertex, KeyVertex * newVertex);
    void updateBoundary_impl(const KeyHalfedge & oldHalfedge, const KeyHalfedge & newHalfedge);
    void updateBoundary_impl(KeyEdge * oldEdge, const KeyEdgeList & newEdges);

    BBox computeBoundingBox_() const { return BBox(0,0,0,0); }

    // Interpolations
    //QList<Eigen::Vector2d> getCoon(Time time, int N);
    //QList<Eigen::Vector2d> getLinear(Time time, int N);

    // -- Inbetween Open Edge --
    Path beforePath_;
    Path afterPath_;
    AnimatedVertex startAnimatedVertex_;
    AnimatedVertex endAnimatedVertex_;

    // -- Inbetween Closed Edge --
    Cycle beforeCycle_;
    Cycle afterCycle_;

// --------- Cloning, Assigning, Copying, Serializing ----------

protected:
    // Cloning
    InbetweenEdge(InbetweenEdge * other);
    virtual InbetweenEdge * clone();
    virtual void remapPointers(VAC * newVAC);

    // Serializing / Unserializing
    virtual QString xmlType_() const;
    virtual void write_(XmlStreamWriter & xml) const;
    InbetweenEdge(VAC * vac, XmlStreamReader &xml);
    virtual void read2ndPass();

    // Serializing / Unserializing DEPRECATED
    virtual void save_(QTextStream & out);
    QString stringType() const {return "InbetweenEdge";}
    InbetweenEdge(VAC * vac, QTextStream & in);
public:
    class Read1stPass {
        friend Cell * Cell::read1stPass(VAC * vac, QTextStream & in);
        static InbetweenEdge * create(VAC * g, QTextStream & in) {return new InbetweenEdge(g, in);}  };
};

        
}

#endif
