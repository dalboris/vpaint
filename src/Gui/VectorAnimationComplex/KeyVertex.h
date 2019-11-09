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

#ifndef VAC_INSTANT_VERTEX_H
#define VAC_INSTANT_VERTEX_H

#include "KeyCell.h"
#include "VertexCell.h"
#include "Halfedge.h"
#include "Eigen.h"

namespace VectorAnimationComplex
{

class KeyVertex: public KeyCell, public VertexCell
{
public:
    // Constructor
    KeyVertex(VAC * vac, Time time, const Eigen::Vector2d & pos);
    KeyVertex(VAC * vac, Time time);

    // Geometry
    void setPos(const Eigen::Vector2d & pos);
    Eigen::Vector2d pos(Time ) const
        { return pos(); }
    Eigen::Vector2d pos() const;
    //double size() const;
    //double size(Time time) const;
    void computePosFromEdges();
    void correctEdgesGeometry();
    Eigen::Vector2d catmullRomTangent(bool slowInOut = false) const;
    Eigen::Vector2d dividedDifferencesTangent(bool slowInOut = false) const;

    // manipulation
    void prepareDragAndDrop();
    void performDragAndDrop(double dx, double dy);
    void prepareAffineTransform();
    void performAffineTransform(const Eigen::Affine2d & xf);

    // For cubic spline interpolation
    KeyVertexList beforeVertices() const;
    KeyVertexList afterVertices() const;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW


private:
        friend class VAC;
    ~KeyVertex();

    // Geometry
    Eigen::Vector2d pos_;
    double size_;

    // Tangents
    QList< QPair< KeyHalfedge, KeyHalfedge> > tangentEdges_;

    // Trusting operators
    friend class Operator;
    bool check_() const;

    // dragAndDrop
    Eigen::Vector2d posBack_;

    void initColor();


// --------- Cloning, Assigning, Copying, Serializing ----------

protected:
    // Cloning
    KeyVertex(KeyVertex * other);
    virtual KeyVertex * clone();
    virtual void remapPointers(VAC * newVAC);

    // Serializing / Unserializing
    virtual QString xmlType_() const;
    virtual void write_(XmlStreamWriter & xml) const;
    KeyVertex(VAC * vac, XmlStreamReader &xml);
    virtual void read2ndPass();

    // Serializing / Unserializing DEPRECATED
    virtual QString stringType() const {return "Vertex";}
    virtual void save_(QTextStream & out);
    KeyVertex(VAC * vac, QTextStream & in);
    public: class Read1stPass {
        friend Cell * Cell::read1stPass(VAC * vac, QTextStream & in);
        static KeyVertex * create(VAC * g, QTextStream & in)
            {return new KeyVertex(g, in);} };
};

}

#endif
