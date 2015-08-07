// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef VAC_ANIMATED_FACE_H
#define VAC_ANIMATED_FACE_H

#include "InbetweenCell.h"
#include "FaceCell.h"

#include "AnimatedCycle.h"

namespace VectorAnimationComplex
{

class InbetweenFace: public InbetweenCell, public FaceCell
{
public:
    InbetweenFace(VAC * vac);
    InbetweenFace(VAC * vac,
                  const QList<AnimatedCycle> & cycles,
                  const QSet<KeyFace*> & beforeFaces,
                  const QSet<KeyFace*> & afterFaces);

    // Topology
    CellSet spatialBoundary() const;
    KeyCellSet beforeCells() const;
    KeyCellSet afterCells() const;

    // Editing cycles
    void addAnimatedCycle(); // invalid cycle
    void addAnimatedCycle(const AnimatedCycle & cycle); // must be valid
    void setCycle(int i, const AnimatedCycle & cycle); // must be valid
    void removeCycle(int i);

    // Editing temporal boundary
    void setBeforeFaces(const QSet<KeyFace*> & beforeFaces);
    void setAfterFaces(const QSet<KeyFace*> & afterFaces);
    void addBeforeFace(KeyFace * beforeFace);
    void addAfterFace(KeyFace * afterFace);
    void removeBeforeFace(KeyFace * beforeFace);
    void removeAfterFace(KeyFace * afterFace);

    // Drawing
    void triangulate(Time time, Triangles & out);

    // Get sampling of the boundary
    QList< QList<Eigen::Vector2d> > getSampling(Time time) const;

    // Getter
    int numAnimatedCycles() const;
    AnimatedCycle animatedCycle(int i) const;
    QSet<KeyFace*> beforeFaces() const;
    QSet<KeyFace*> afterFaces() const;

private:
    // Trusting operators
    friend class VAC;
    friend class Operator;
    bool check_() const;

    ~InbetweenFace();

    // Update Boundary
    void updateBoundary_impl(KeyVertex * oldVertex, KeyVertex * newVertex);
    void updateBoundary_impl(const KeyHalfedge & oldHalfedge, const KeyHalfedge & newHalfedge);
    void updateBoundary_impl(KeyEdge * oldEdge, const KeyEdgeList & newEdges);

    // Cycles
    QList<AnimatedCycle> cycles_;
    QSet<KeyFace*> beforeFaces_;
    QSet<KeyFace*> afterFaces_;

    BBox computeBoundingBox_() const { return BBox(0,0,0,0); }

// --------- Cloning, Assigning, Copying, Serializing ----------

protected:
    // Cloning
    InbetweenFace(InbetweenFace * other);
    virtual InbetweenFace * clone();
    virtual void remapPointers(VAC * newVAC);

    // Serializing / Unserializing
    virtual QString xmlType_() const;
    virtual void write_(XmlStreamWriter & xml) const;
    InbetweenFace(VAC * vac, XmlStreamReader &xml);
    virtual void read2ndPass();

    // Serializing / Unserializing DEPRECATED
    virtual void save_(QTextStream & out);
    QString stringType() const {return "InbetweenFace";}
    InbetweenFace(VAC * vac, QTextStream & in);
      public: class Read1stPass {
    friend Cell * Cell::read1stPass(VAC * vac, QTextStream & in);
    static InbetweenFace * create(VAC * g, QTextStream & in)
            {return new InbetweenFace(g, in);}  };
private:
    QSet<int> tempBeforeFaces_;
    QSet<int> tempAfterFaces_;
};

}

#endif
