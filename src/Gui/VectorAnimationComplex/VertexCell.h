// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef VAC_VERTEX_OBJECT_H
#define VAC_VERTEX_OBJECT_H

#include "Eigen.h"
#include "Cell.h"
#include "Halfedge.h"
#include <QPair>

namespace VectorAnimationComplex
{

class VertexCell: virtual public Cell
{
public:
    VertexCell(VAC * vac);

    // Casting
    VertexCell * toVertexCell() { return this; }

    // Geometry
    virtual Eigen::Vector2d pos(Time time) const=0;
    double size(Time time) const; // Note: computed from incident edges

    // Drawing
    //void draw(Time time, ViewSettings & viewSettings);
    void drawRaw(Time time, ViewSettings & viewSettings);
    void drawRawTopology(Time time, ViewSettings & viewSettings);
    void drawEdgeJunction(Time time, ViewSettings & viewSettings);

    // Topology
    CellSet spatialBoundary() const;
    CellSet spatialBoundary(Time t) const;
    QList<Halfedge> incidentEdges(Time t) const;


protected:
    virtual ~VertexCell()=0;

private:

    // Trusting operators
    friend class Operator;
    bool checkVertex_() const;

    void drawPickCustom(Time time, ViewSettings & viewSettings);
    bool isPickableCustom(Time time) const;

    // Implementation of triangulate for both KeyVertex and InbetweenVertex
    void triangulate_(Time time, Triangles & out);

// --------- Cloning, Assigning, Copying, Serializing ----------

protected:
    virtual void save_(QTextStream & out);
    VertexCell(VertexCell * other);
    VertexCell(VAC * vac, QTextStream & in);
    virtual QString stringType() const {return "VertexCell";}
    virtual void read2ndPass();
    virtual void remapPointers(VAC * newVAC);
    VertexCell(VAC * vac, XmlStreamReader & xml);
    virtual void write_(XmlStreamWriter & xml) const;
};

}


#endif
 
