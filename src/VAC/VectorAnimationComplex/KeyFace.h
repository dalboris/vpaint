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

#ifndef VAC_INSTANT_FACE_H
#define VAC_INSTANT_FACE_H

#include "KeyCell.h"
#include "FaceCell.h"
#include "ProperCycle.h"
#include "CycleHelper.h"
#include "Cycle.h"
#include "Triangles.h"

namespace VectorAnimationComplex
{

class KeyFace: public KeyCell, public FaceCell
{
public:
    // Pre-conditions:
    //   - vac must not be NULL
    KeyFace(VAC * vac, const Time & t);

    // Pre-conditions:
    //   - vac must not be NULL
    //   - cycle must be valid
    //   - cells of cycle must belong to vac
    KeyFace(VAC * vac, const Cycle & cycle);

    // Pre-conditions:
    //   - vac must not be NULL
    //   - there must be at least one cycle
    //   - all cycles must be valid
    //   - cells of cycles must belong to vac
    KeyFace(VAC * vac, const QList<Cycle> & cycles);

    // Pre-conditions:
    //   - cycle must be valid
    //   - cells of cycle must belong to vac()
    void addCycle(const Cycle & cycle);

    // Pre-conditions:
    //   - all cycles must be valid
    //   - cells of cycles must belong to vac()
    void addCycles(const QList<Cycle> & cycles);

    // Pre-conditions:
    //   - all cycles must be valid
    //   - there must be at least one cycle
    //   - cells of cycles must belong to vac()
    void setCycles(const QList<Cycle> & cycles);

    // Drawing

    // Get sampling of the boundary
    QList< QList<Eigen::Vector2d> > getSampling(Time time) const;

    // Boundary
    CellSet spatialBoundary() const;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:
    friend class VAC;
    virtual ~KeyFace();

    // Constructor helper
    void initColor_();

    // Trusting operators
    friend class Operator;
    bool check_() const;

    // Update Boundary
    void updateBoundary_impl(KeyVertex * oldVertex, KeyVertex * newVertex);
    void updateBoundary_impl(const KeyHalfedge & oldHalfedge, const KeyHalfedge & newHalfedge);
    void updateBoundary_impl(KeyEdge * oldEdge, const KeyEdgeList & newEdges);

    // Cycles
    QList<Cycle> cycles_;

    // Remove all cycles.
    void clearCycles_();

    // Implementation of triangulate
    void triangulate_(Time time, Triangles & out) const;

// --------- Cloning, Assigning, Copying, Serializing ----------

protected:
    // Cloning
    KeyFace(KeyFace * other);
    virtual KeyFace * clone();
    virtual void remapPointers(VAC * newVAC);

    // Serializing
    virtual void save_(QTextStream & out);
    QString stringType() const {return "Face";}
    virtual QString xmlType_() const;
    virtual void write_(XmlStreamWriter & xml) const;


    // Unserializing
    KeyFace(VAC * vac, QTextStream & in);
    KeyFace(VAC * vac, XmlStreamReader & xml);
      public: class Read1stPass {
    friend Cell * Cell::read1stPass(VAC * vac, QTextStream & in);
    static KeyFace * create(VAC * g, QTextStream & in)
            {return new KeyFace(g, in);}  };
      protected: virtual void read2ndPass();

};

class PreviewKeyFace
{
public:
    PreviewKeyFace();
    PreviewKeyFace(const Cycle & cycle);
    PreviewKeyFace(const QList<Cycle> & cycles);

    // Clear
    void clear();

    // Append cycle
    PreviewKeyFace & operator<< (const Cycle & cycle);

    // Draw
    void draw(ViewSettings & viewSettings);

    // Cycles
    int numCycles() const {return cycles_.size();}
    QList<Cycle> cycles() {return cycles_;}

    // Test whether the point (x,y) is within the face
    bool intersects(double x, double y) const;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:
    QList<Cycle> cycles_;

    void computeTriangles_();
    Triangles triangles_;
};

}

#endif
