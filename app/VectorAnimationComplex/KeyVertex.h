// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

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

    // Bounding box
    BBox computeBoundingBox_() const;


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
