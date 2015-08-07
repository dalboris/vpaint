// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef VAC_EDGE_CELL_H
#define VAC_EDGE_CELL_H

#include "Cell.h"
#include "Triangles.h"
#include "EdgeSample.h"

#include <QMap>

namespace VectorAnimationComplex
{

class EdgeCell: virtual public Cell
{
public:
    EdgeCell(VAC * vac);

    // Check if the edge is a topological loop
    virtual bool isClosed() const = 0;

    // Boundary
    CellSet spatialBoundary() const;
    virtual VertexCellSet startVertices() const=0;
    virtual VertexCellSet endVertices() const=0;

    // Specific topological relations
    EdgeCellSet incidentEdges() const;

    // Drawing
    virtual void triangulate(Time time, Triangles & out);
    virtual void triangulate(double width, Time time, Triangles & out);
    Triangles & triangles(Time time);
    Triangles & triangles(double width, Time time);
    void drawRaw(Time time, ViewSettings & viewSettings);
    void drawRawTopology(Time time, ViewSettings & viewSettings);

    // Geometric getters
    virtual QList<EdgeSample> getSampling(Time time) const = 0;
    virtual EdgeSample startSample(Time time) const;
    virtual EdgeSample endSample(Time time) const;

    // Intersection test
    virtual bool intersectsRectangle(Time time, double x0, double x1, double y0, double y1);

    // Export SVG
    virtual void exportSVG(Time t, QTextStream & out);

protected:
    // Cached triangulations (the integer represent a 1/60th of frame)
    QMap<int,Triangles> triangles_;
    QMap< QPair<int,double>, Triangles> trianglesTopo_; // (int=time, double=width)
    virtual void clearCachedGeometry_();

private:
    // Trusting operators
    friend class Operator;
    bool checkEdge_() const;

    virtual bool isPickableCustom(Time time) const;

protected:
    //void split_copySpatialStarTo(EdgeCell * cell);
    //void split_clearSpatialStar();

    virtual ~EdgeCell()=0;


// --------- Cloning, Assigning, Copying, Serializing ----------

protected:
    virtual void remapPointers(VAC * newVAC);
    virtual void save_(QTextStream & out);
    EdgeCell(EdgeCell * other);
    virtual QString stringType() const {return "EdgeCell";}
    EdgeCell(VAC * vac, QTextStream & in);
    virtual void read2ndPass();
    virtual void write_(XmlStreamWriter & xml) const;
    EdgeCell(VAC * vac, XmlStreamReader & xml);
};

}


#endif
