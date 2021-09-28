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

#ifndef VAC_VERTEX_OBJECT_H
#define VAC_VERTEX_OBJECT_H

#include "Eigen.h"
#include "Cell.h"
#include "Halfedge.h"
#include <QPair>
#include "vpaint_global.h"

namespace VectorAnimationComplex
{

class Q_VPAINT_EXPORT VertexCell: virtual public Cell
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
    void triangulate_(Time time, Triangles & out) const;

    // Implementation of outline bounding box for both KeyVertex and InbetweenVertex
    void computeOutlineBoundingBox_(Time t, BoundingBox & out) const;

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
 
