// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef INSTANTHOLE_H
#define INSTANTHOLE_H

#include "ProperPath.h"
#include "ProperCycle.h"

class KeyVertex;
class QTextStream;

////////////// Forward declare global serialization operators /////////////////

namespace VectorAnimationComplex { class CycleHelper; }
QTextStream & operator<<(QTextStream &, const VectorAnimationComplex::CycleHelper &);
QTextStream & operator>>(QTextStream &, VectorAnimationComplex::CycleHelper &);

///////////////////////////////////////////////////////////////////////////////


namespace VectorAnimationComplex
{

class CycleHelper
{
public:
    // Invalid hole
    CycleHelper();

    // Create a hole that is a single vertex
    CycleHelper(KeyVertex * vertex);

    // create a hole that is not a single vertex
    CycleHelper(const KeyEdgeSet & edgeSet);

    // type of the hole
    bool isValid() const;
    bool isSingleVertex() const;

    // all methods below assume that the hole is valid

    // Time
    Time time() const;

    // In case of single vertex (can be call a Steiner vertex)
    KeyVertex * vertex() const;

    // In case of not a single vertex
    int nLoops() const;
    const ProperCycle & loop(int i) const;
    int nPaths() const;
    const ProperPath & path(int i) const;

    // serialization and copy
    void remapPointers(VAC * newVAC);
    friend QTextStream & ::operator<<(QTextStream & out, const CycleHelper & hole);
    friend QTextStream & ::operator>>(QTextStream & in, CycleHelper & hole);
    void convertTempIdsToPointers(VAC * vac);


    // The set of cells this helper points to
    KeyCellSet cells() const;

    // Replace pointed edges
    void replaceEdges(KeyEdge * oldEdge, const KeyEdgeList & newEdges);

private:
    // Single vertex (==null if the hole is not a single vertex)
    KeyVertex * singleVertex_;
    int tempId_;

    // Internal holes
    QList<ProperCycle> loops_;

    // External cracks
    QList<ProperPath> paths_;
};

} // end namespace VectorAnimationComplex

#endif // INSTANTHOLE_H
