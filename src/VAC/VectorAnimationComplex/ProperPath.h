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

#ifndef INSTANTPATH_H
#define INSTANTPATH_H

#include "../TimeDef.h"

#include <QList>
#include "KeyHalfedge.h"
#include "VAC/vpaint_global.h"

////////////// Forward declare global serialization operators /////////////////

namespace VectorAnimationComplex { class ProperPath; }
QTextStream & operator<<(QTextStream &, const VectorAnimationComplex::ProperPath &);
QTextStream & operator>>(QTextStream &, VectorAnimationComplex::ProperPath &);

///////////////////////////////////////////////////////////////////////////////


namespace VectorAnimationComplex
{

// A proper path is a list of n>1 consecutive open halfeges satisfying:
//  * no repeated halfedge
//  * startVertex() != endVertex()
class Q_VPAINT_EXPORT ProperPath
{
public:
    // invalid path
    ProperPath();

    // edges: unsorted set of instant edges
    ProperPath(const KeyEdgeSet & edgeSet);

    bool isValid() const;

    // all methods below assume that the loop is valid
    Time time() const;
    int size() const;
    KeyHalfedge operator[](int i) const;

    // serialization and copy
    void remapPointers(VAC * newVAC);
    void save(QTextStream & out);
    friend QTextStream & ::operator<<(QTextStream & out, const ProperPath & path);
    friend QTextStream & ::operator>>(QTextStream & in, ProperPath & path);
    void convertTempIdsToPointers(VAC * vac);

    // Replace pointed edges
    void replaceEdges(KeyEdge * oldEdge, const KeyEdgeList & newEdges);

private:
    // sorted list of instant edges, guaranted to share the same time
    QList<KeyHalfedge> halfedges_;
};

} // end namespace VectorAnimationComplex


#endif // INSTANTPATH_H
