// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef ANIMATEDCYCLE_H
#define ANIMATEDCYCLE_H

#include "Cell.h"
#include "CellList.h"
#include "../TimeDef.h"
#include "Eigen.h"
#include <QList>

////////////// Forward declare global serialization operators /////////////////

namespace VectorAnimationComplex { class AnimatedCycle; }
QTextStream & operator<<(QTextStream &, const VectorAnimationComplex::AnimatedCycle &);
QTextStream & operator>>(QTextStream &, VectorAnimationComplex::AnimatedCycle &);

///////////////////////////////////////////////////////////////////////////////

namespace VectorAnimationComplex
{

// A node of the animated cycle. Note: it is the AnimatedCycle responsibility
// to ensure consistency.
class AnimatedCycleNode
{
public:
    AnimatedCycleNode(Cell * cell);

    // Type
    enum NodeType
    {
        InvalidNode,
        KeyVertexNode,
        KeyOpenEdgeNode,
        KeyClosedEdgeNode,
        InbetweenVertexNode,
        InbetweenOpenEdgeNode,
        InbetweenClosedEdgeNode
    };
    NodeType nodeType() const;
    enum CycleType
    {
        InvalidCycle,
        SteinerCycle,
        SimpleCycle,
        NonSimpleCycle
    };
    CycleType cycleType(Time time) const;

    // Setters
    void setCell(Cell * cell);
    void setPrevious(AnimatedCycleNode * node);
    void setNext(AnimatedCycleNode * node);
    void setBefore(AnimatedCycleNode * node);
    void setAfter(AnimatedCycleNode * node);

    // Getters
    Cell * cell() const;
    AnimatedCycleNode * previous() const;
    AnimatedCycleNode * next() const;
    AnimatedCycleNode * before() const;
    AnimatedCycleNode * after() const;

    // Spatial cycling
    AnimatedCycleNode * previous(Time time) const;
    AnimatedCycleNode * next(Time time) const;

    // For halfedges
    bool side() const;
    void setSide(bool side);

private:
    Cell * cell_;
    AnimatedCycleNode * previous_;
    AnimatedCycleNode * next_;
    AnimatedCycleNode * before_;
    AnimatedCycleNode * after_;
    bool side_;
};

class AnimatedCycle
{
public:
    AnimatedCycle();
    AnimatedCycle(AnimatedCycleNode * first); // It's the caller responsibility to allocate and create the nodes
                                              // node ownership is transfered to the cycle (unless you know what you're doing)
    AnimatedCycle(const AnimatedCycle & other);
    AnimatedCycle & operator=(const AnimatedCycle & other);
    ~AnimatedCycle();

    // First node
    AnimatedCycleNode  * first() const;
    void setFirst(AnimatedCycleNode  * node);

    // Find a node at particular time
    AnimatedCycleNode  * getNode(Time time);

    // Find all noded refering to particular cell
    QSet<AnimatedCycleNode*> getNodes(Cell * cell);

    // Find all nodes
    QSet<AnimatedCycleNode*> nodes() const; // Note: only return nodes connected to first_ (i.e., may not work if cycle is invalid)

    // Find all cells
    CellSet cells() const;
    KeyCellSet beforeCells() const; // temporal boundary of n->before == NULL
    KeyCellSet afterCells() const; // temporal boundary of n->after == NULL

    // Geometry
    void sample(Time time, QList<Eigen::Vector2d> & out);

    // Replace pointed vertex
    void replaceVertex(KeyVertex * oldVertex, KeyVertex * newVertex);
    void replaceHalfedge(const KeyHalfedge & oldHalfedge, const KeyHalfedge & newHalfedge);
    void replaceEdges(KeyEdge * oldEdge, const KeyEdgeList & newEdges);
    void replaceInbetweenVertex(InbetweenVertex * sv, // old
                                InbetweenVertex * sv1, KeyVertex * kv, InbetweenVertex * sv2); // new
    void replaceInbetweenEdge(InbetweenEdge * se, // old
                                InbetweenEdge * se1, KeyEdge *ke, InbetweenEdge * se2); // new

    // serialization and copy
    void remapPointers(VAC * newVAC);
    friend QTextStream & ::operator<<(QTextStream & out, const AnimatedCycle & cycle);
    friend QTextStream & ::operator>>(QTextStream & in, AnimatedCycle & cycle);
    void convertTempIdsToPointers(VAC * vac);
    QString toString() const;
    void fromString(const QString & str);

    // Methods that can make the animated cycle invalid. Use with caution


private:
    void clear();
    void copyFrom(const AnimatedCycle & other);

    AnimatedCycleNode * first_;

    // for unserialization
    friend class InbetweenFace;
    struct TempNode { int cell, previous, next, before, after; bool side; };
    QList<TempNode> tempNodes_;
};

} // end namespace VectorAnimationComplex

#endif // ANIMATEDCYCLE_H
