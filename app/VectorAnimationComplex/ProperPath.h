// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef INSTANTPATH_H
#define INSTANTPATH_H

#include "../TimeDef.h"

#include <QList>
#include "KeyHalfedge.h"

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
class ProperPath
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
