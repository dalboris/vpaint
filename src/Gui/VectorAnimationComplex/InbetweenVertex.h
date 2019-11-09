// Copyright (C) 2012-2019 The VPaint Developers
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

#ifndef VAC_ANIMATED_VERTEX_H
#define VAC_ANIMATED_VERTEX_H

#include "Eigen.h"
#include "InbetweenCell.h"
#include "VertexCell.h"
#include <QList>

namespace VectorAnimationComplex
{

class InbetweenVertex: public InbetweenCell, public VertexCell
{
public:
    // Constructor
    InbetweenVertex(VAC * vac,
             KeyVertex * beforeVertex,
             KeyVertex * afterVertex);

    // Topology
    KeyCellSet beforeCells() const;
    KeyCellSet afterCells() const;
    KeyVertex * beforeVertex() const;
    KeyVertex * afterVertex() const;

    //Drawing
    void glColor3D_();
    void drawRaw3D(View3DSettings & viewSettings);

    // Geometry
    Eigen::Vector2d pos(Time time) const;
    //double size(Time time) const;



    // Operators
    /*
    void addEdgeNeighbour(Edge * edge, EdgeBoundary b);
    void removeEdgeNeighbour(Edge * edge, EdgeBoundary b);
    */
    //KeyCell *insertKeyFrame(Time time);


private:
    // Trusting operators
    friend class VAC;
    friend class Operator;
    bool check_() const;

    ~InbetweenVertex();

    // Update Boundary
    void updateBoundary_impl(KeyVertex * oldVertex, KeyVertex * newVertex);

    KeyVertex * beforeVertex_;
    KeyVertex * afterVertex_;

    // Cubic spline interpolation
    //KeyVertexList beforeBeforenodes() const;
    //KeyVertexList afterAfternodes() const;
    Eigen::Vector2d posCubic(Time time) const;

    // Linear interpolation
    Eigen::Vector2d posLinear(Time time) const;

// --------- Cloning, Assigning, Copying, Serializing ----------

protected:
    // Cloning
    InbetweenVertex(InbetweenVertex * other);
    virtual InbetweenVertex * clone();
    virtual void remapPointers(VAC * newVAC);

    // Serializing / Unserializing
    virtual QString xmlType_() const;
    virtual void write_(XmlStreamWriter & xml) const;
    InbetweenVertex(VAC * vac, XmlStreamReader &xml);
    virtual void read2ndPass();

    // Serializing / Unserializing DEPRECATED
    virtual void save_(QTextStream & out);
    virtual QString stringType() const {return "InbetweenVertex";}
    InbetweenVertex(VAC * vac, QTextStream & in);
      public: class Read1stPass {
    friend Cell * Cell::read1stPass(VAC * vac, QTextStream & in);
    static InbetweenVertex * create(VAC * g, QTextStream & in)
            {return new InbetweenVertex(g, in);}  };

private:
    struct TempRead { int before, after; };
    TempRead * tmp_;

};
    
}

#endif
