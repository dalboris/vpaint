// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef PATH_H
#define PATH_H

#include "../TimeDef.h"

#include <QList>
#include "KeyHalfedge.h"
#include "ProperPath.h"
#include "ProperCycle.h"

////////////// Forward declare global serialization operators /////////////////

namespace VectorAnimationComplex { class Path; }
QTextStream & operator<<(QTextStream &, const VectorAnimationComplex::Path &);
QTextStream & operator>>(QTextStream &, VectorAnimationComplex::Path &);

///////////////////////////////////////////////////////////////////////////////

namespace VectorAnimationComplex
{

class Path
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
