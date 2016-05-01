// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

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
