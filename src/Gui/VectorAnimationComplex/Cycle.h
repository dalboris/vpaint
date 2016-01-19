// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef INSTANTCYCLE_H
#define INSTANTCYCLE_H

#include "../TimeDef.h"

#include <QList>
#include "KeyHalfedge.h"
#include "Eigen.h"
#include "ProperCycle.h"
#include "EdgeSample.h"

////////////// Forward declare global serialization operators /////////////////

namespace VectorAnimationComplex { class Cycle; }
QTextStream & operator<<(QTextStream &, const VectorAnimationComplex::Cycle &);
QTextStream & operator>>(QTextStream &, VectorAnimationComplex::Cycle &);

///////////////////////////////////////////////////////////////////////////////

namespace VectorAnimationComplex
{

class Cycle
{
public:
    // type of the cycle
    enum CycleType
    {
        Invalid,
        SingleVertex,
        ClosedHalfedge,
        OpenHalfedgeList
    };
    CycleType type() const;

    // invalid loop
    Cycle();

    // try to build a vertex type of cycle
    Cycle(KeyVertex * instantVertex);

    // try to build a closed halfedge or open halfedge list type of cycle
    Cycle(const KeyEdgeSet & edgeSet);

    // if you already know the cycle
    Cycle(const QList<KeyHalfedge> & halfedges);

    // Conversion from proper cycle
    Cycle(const ProperCycle & properCycle);

    bool isValid() const;

    // all methods below assume that the loop is valid, and that you request something meaningful according to type
    Time time() const;
    KeyVertex * singleVertex() const;
    KeyHalfedge closedHalfedge() const;
    int size() const;
    KeyHalfedge operator[](int i) const;

    // The set of cells this helper points to
    KeyCellSet cells() const;

    // serialization and copy
    void remapPointers(VAC * newVAC);
    friend QTextStream & ::operator<<(QTextStream & out, const Cycle & cycle);
    friend QTextStream & ::operator>>(QTextStream & in, Cycle & cycle);
    void convertTempIdsToPointers(VAC * vac);
    QString toString() const;
    void fromString(const QString & str);

    // Replace boundary cells by other cells
    void replaceVertex(KeyVertex * oldVertex, KeyVertex * newVertex);
    void replaceHalfedge(const KeyHalfedge & oldHalfedge, const KeyHalfedge & newHalfedge);
    void replaceEdges(KeyEdge * oldEdge, const KeyEdgeList & newEdges);

    // geometry
    double length() const;

    void sample(QList<Eigen::Vector2d> & out) const; // Note: out[0] == out[n-1]
    void sample(int numSamples, QList<Eigen::Vector2d> & out) const;
    void sample(int numSamples, QList<EdgeSample> & out) const;

    // Curvature-related methods
    double totalCurvature() const;
    int turningNumber() const;

    // Getters
    double s0() const;

    // Setters
    void setStartingPoint(double s0);

    // Reversed cycle
    Cycle reversed() const;


private:
    friend class VAC;

    // vertex
    int tempId_;
    KeyVertex * vertex_;

    // started point
    double s0_;

    // sorted list of instant edges
    QList<KeyHalfedge> halfedges_;
};

} // end namespace VectorAnimationComplex

#endif // INSTANTCYCLE_H
