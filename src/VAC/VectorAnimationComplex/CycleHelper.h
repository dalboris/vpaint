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

#ifndef INSTANTHOLE_H
#define INSTANTHOLE_H

#include "ProperPath.h"
#include "ProperCycle.h"
#include "VAC/vpaint_global.h"

class QTextStream;

////////////// Forward declare global serialization operators /////////////////

namespace VectorAnimationComplex { class CycleHelper; }
QTextStream & operator<<(QTextStream &, const VectorAnimationComplex::CycleHelper &);
QTextStream & operator>>(QTextStream &, VectorAnimationComplex::CycleHelper &);

///////////////////////////////////////////////////////////////////////////////


namespace VectorAnimationComplex
{

class Q_VPAINT_EXPORT CycleHelper
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
