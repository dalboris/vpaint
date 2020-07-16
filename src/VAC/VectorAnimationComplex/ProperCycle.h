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
