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

#ifndef VAC_INSTANT_EDGE_H
#define VAC_INSTANT_EDGE_H

#include "EdgeCell.h"
#include "KeyCell.h"
#include "Eigen.h"
#include "Triangles.h"
#include "VAC/vpaint_global.h"

namespace VectorAnimationComplex
{
class EdgeGeometry;
class EdgeInter;
class IntersectionList;

class Q_VPAINT_EXPORT KeyEdge: public KeyCell, public EdgeCell
{
public:

    // Constructor. Geometry must be a valid non-null pointer.
    // KeyEdge takes ownership of geometry. The rationale is to let the
    // caller specify the actual derived class used for geometry, for instance:
    // new InstanceEdge(this, time, left, right, new LinearSpline(...));
    // non loop
    KeyEdge(VAC * vac, Time time,
         KeyVertex * startVertex,
         KeyVertex * endVertex,
         EdgeGeometry * geometry);
    // loop
    KeyEdge(VAC * vac, Time time,
         EdgeGeometry * geometry);

    // Drawing
    virtual void drawPickTopology(Time time, ViewSettings & viewSettings);
    void draw3DSmall();
    void drawRaw3D(View3DSettings & viewSettings);

    // Topology
    KeyVertex * startVertex() const { return startVertex_; }
    KeyVertex * endVertex() const { return endVertex_; }
    bool isSplittedLoop() const
        { return (!isClosed()) && (startVertex_ == endVertex_); }
    bool isClosed() const { return !startVertex_; }

    // reimplements
    VertexCellSet startVertices() const;
    VertexCellSet endVertices() const;


    // Geometry
    EdgeGeometry * geometry() const { return geometry_; }
    void correctGeometry();
    void setWidth(double newWidth);
    QList<EdgeSample> getSampling(Time time) const;


    // Sculpting
    // prepare edge for potential sculpting:
    //    - (x,y) is mouse position
    //    - radius is the radius of influence of the sculpt tool
    //    - must return the distance from (x,y) to the point where it would be sculpted.
    //    - may store all relevant info to provide sculptVertex() later.
    double updateSculpt(double x, double y, double radius);
    // Deform
    void beginSculptDeform(double x, double y);
    void continueSculptDeform(double x, double y);
    void endSculptDeform();
    // Change edge width
    void beginSculptEdgeWidth(double x, double y);
    void continueSculptEdgeWidth(double x, double y);
    void endSculptEdgeWidth();
    // Change edge width
    void beginSculptSmooth(double x, double y);
    void continueSculptSmooth(double x, double y);
    void endSculptSmooth();
    // Affine transform
    void prepareAffineTransform();
    void performAffineTransform(const Eigen::Affine2d & xf);

private:
    friend class VAC;

    ~KeyEdge();
    KeyVertex * startVertex_;
    KeyVertex * endVertex_;
    EdgeGeometry * geometry_;

    // Trusting operators
    friend class Operator;
    bool check_() const;

    // Update Boundary
    void updateBoundary_impl(KeyVertex * oldVertex, KeyVertex * newVertex);

    // Note:  topFace  or  bottom  can be  either  instant  or
    // animated, at  the contrary of InbetweenEdge  where it is
    // necessarily animated, and  hence have a specific member
    // to  store  it,  in   addition  to  the  one  stored  in
    // EdgeObject base class

    // for sculpting
    void prepareSculptPreserveTangents_();
    void continueSculptPreserveTangents_();
    KeyEdgeSet sculpt_keepRightAsLeft_;
    KeyEdgeSet sculpt_keepLeftAsLeft_;
    KeyEdgeSet sculpt_keepLeftAsRight_;
    KeyEdgeSet sculpt_keepRightAsRight_;
    Eigen::Vector2d sculpt_beginLeftDer_;
    Eigen::Vector2d sculpt_beginRightDer_;
    bool sculpt_keepMyselfTangent_;
    double sculptRadius_;
    double remainingRadiusLeft_;
    double remainingRadiusRight_;

    // Implementation of triangulate
    void triangulate_(Time time, Triangles & out) const;
    void triangulate_(double width, Time time, Triangles & out) const;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW


// --------- Cloning, Assigning, Copying, Serializing ----------

protected:
    // Cloning
    KeyEdge(KeyEdge * other);
    virtual KeyEdge * clone();
    virtual void remapPointers(VAC * newVAC);

    // Serializing
    virtual void save_(QTextStream & out);
    QString stringType() const {return "Edge";}
    virtual QString xmlType_() const;
    virtual void write_(XmlStreamWriter & xml) const;


    // Unserializing
    KeyEdge(VAC * vac, XmlStreamReader & xml);
    KeyEdge(VAC * vac, QTextStream & in);
      public: class Read1stPass {
    friend Cell * Cell::read1stPass(VAC * vac, QTextStream & in);
    static KeyEdge * create(VAC * g, QTextStream & in)
            {return new KeyEdge(g, in);}  };
      protected: virtual void read2ndPass();
private:
    struct TempRead { int left, right; };
    TempRead * tmp_;
};

}

#endif
