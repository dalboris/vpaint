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
    const Triangles & triangles(double width, Time time) const;
    void drawRawTopology(Time time, ViewSettings & viewSettings);

    // Geometric getters
    virtual QList<EdgeSample> getSampling(Time time) const = 0;
    virtual EdgeSample startSample(Time time) const;
    virtual EdgeSample endSample(Time time) const;

    // Export SVG
    virtual void exportSVG(QTextStream & out, const VectorExportSettings & settings, Time t);

protected:
    // Special handling to draw edges of fixed screen-width in topology mode
    // (int=time, double=width)
    mutable QMap< QPair<int,double>, Triangles> trianglesTopo_;
    virtual void clearCachedGeometry_();
    virtual void triangulate_(double width, Time time, Triangles & out) const=0;

private:
    // Trusting operators
    friend class Operator;
    bool checkEdge_() const;

    virtual bool isPickableCustom(Time time) const;

    // Implementation of outline bounding box for both KeyVertex and InbetweenVertex
    void computeOutlineBoundingBox_(Time t, BoundingBox & out) const;

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
