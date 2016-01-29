// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

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
    using Cell::triangles;
    Triangles & triangles(double width, Time time);
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
    // Special handling to draw edges of fixed screen-width in topology mode
    // (int=time, double=width)
    QMap< QPair<int,double>, Triangles> trianglesTopo_;
    virtual void clearCachedGeometry_();
    virtual void triangulate_(double width, Time time, Triangles & out)=0;

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
