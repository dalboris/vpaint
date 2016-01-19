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

#ifndef INSTANTLOOP_H
#define INSTANTLOOP_H

#include "../TimeDef.h"

#include <QList>
#include "KeyHalfedge.h"

////////////// Forward declare global serialization operators /////////////////

namespace VectorAnimationComplex { class ProperCycle; }
QTextStream & operator<<(QTextStream &, const VectorAnimationComplex::ProperCycle &);
QTextStream & operator>>(QTextStream &, VectorAnimationComplex::ProperCycle &);

///////////////////////////////////////////////////////////////////////////////

namespace VectorAnimationComplex
{
// A proper path is either a list of n>1 consecutive open halfeges satisfying:
//  * no repeated halfedge
//  * startVertex() == endVertex()
// Or a single closed halfedge
class ProperCycle
{
public:
    // invalid loop
    ProperCycle();

    // edges: unsorted set of instant edges
    ProperCycle(const KeyEdgeSet & edgeSet);

    bool isValid() const;

    // all methods below assume that the loop is valid
    Time time() const;
    int size() const;
    KeyHalfedge operator[](int i) const;

    // serialization and copy
    void remapPointers(VAC * newVAC);
    friend QTextStream & ::operator<<(QTextStream & out, const ProperCycle & loop);
    friend QTextStream & ::operator>>(QTextStream & in, ProperCycle & loop);
    void convertTempIdsToPointers(VAC * vac);

    // Replace pointed edges
    void replaceEdges(KeyEdge * oldEdge, const KeyEdgeList & newEdges);

private:
    // sorted list of instant edges, guaranted to share the same time
    QList<KeyHalfedge> halfedges_;
};

} // end namespace VectorAnimationComplex

#endif // INSTANTLOOP_H
