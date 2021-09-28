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

#ifndef PATH_H
#define PATH_H

#include "../TimeDef.h"

#include <QList>
#include "KeyHalfedge.h"
#include "ProperPath.h"
#include "ProperCycle.h"
#include "vpaint_global.h"
////////////// Forward declare global serialization operators /////////////////

namespace VectorAnimationComplex { class Path; }
QTextStream & operator<<(QTextStream &, const VectorAnimationComplex::Path &);
QTextStream & operator>>(QTextStream &, VectorAnimationComplex::Path &);

///////////////////////////////////////////////////////////////////////////////

namespace VectorAnimationComplex
{

class Q_VPAINT_EXPORT Path
{
public:
    // type of the Path
    enum PathType
    {
        Invalid,
        SingleVertex,
        OpenHalfedgeList
    };
    PathType type() const;

    // invalid path
    Path();

    // try to build a vertex type of Path
    Path(KeyVertex * instantVertex);

    // try to build a closed halfedge or open halfedge list type of Path
    Path(const KeyEdgeSet & edgeSet);

    // if you already know the non-simple Path
    Path(const QList<KeyHalfedge> halfedges);

    // Conversion from proper path
    Path(const ProperPath & properPath); // always return a valid path unless properPath is invalid

    // Conversion from proper cycle
    Path(const ProperCycle & properCycle); // may return an invalid path even if properCycle is valid
                                           // this happens when properCycle is made of a single closed edge
    bool isValid() const;

    // Getters
    KeyVertex * startVertex() const;
    KeyVertex * endVertex() const;

    // all methods below assume that the path is valid, and that you request something meaningful according to type
    Time time() const;
    KeyVertex * singleVertex() const;
    int size() const;
    KeyHalfedge operator[](int i) const;

    // The set of cells this helper points to
    KeyCellSet cells() const;

    // serialization and copy
    void remapPointers(VAC * newVAC);
    friend QTextStream & ::operator<<(QTextStream & out, const Path & Path);
    friend QTextStream & ::operator>>(QTextStream & in, Path & Path);
    void convertTempIdsToPointers(VAC * vac);
    QString toString() const;
    void fromString(const QString & str);

    // Replace boundary cells by other cells
    void replaceVertex(KeyVertex * oldVertex, KeyVertex * newVertex);
    void replaceHalfedge(const KeyHalfedge & oldHalfedge, const KeyHalfedge & newHalfedge);
    void replaceEdges(KeyEdge * oldEdge, const KeyEdgeList & newEdges);

    // geometry
    double length() const;
    void sample(int numSamples, QList<Eigen::Vector2d> & out) const;
    void sample(int numSamples, QList<EdgeSample> & out) const;

    // Reversed path
    Path reversed() const;

private:
    friend class VAC;

    // vertex
    int tempId_;
    KeyVertex * vertex_;

    // sorted list of instant edges
    QList<KeyHalfedge> halfedges_;
};

} // end namespace VectorAnimationComplex

#endif // PATH_H
