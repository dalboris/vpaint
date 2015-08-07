// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef ANIMATEDVERTEX_H
#define ANIMATEDVERTEX_H

#include "CellList.h"
#include "Eigen.h"
#include "../TimeDef.h"

////////////// Forward declare global serialization operators /////////////////

namespace VectorAnimationComplex { class AnimatedVertex; }
QTextStream & operator<<(QTextStream &, const VectorAnimationComplex::AnimatedVertex &);
QTextStream & operator>>(QTextStream &, VectorAnimationComplex::AnimatedVertex &);

///////////////////////////////////////////////////////////////////////////////

namespace VectorAnimationComplex
{

class AnimatedVertex
{
public:
    // Construct an invalid animated vertex
    AnimatedVertex();

    // Construct an animated vertex, assume it is valid
    AnimatedVertex(const InbetweenVertexList & inbetweenVertices);

    // Check validity
    bool isValid() const;

    // Getters
    InbetweenVertexList inbetweenVertices() const;

    KeyVertex * beforeVertex() const;
    KeyVertex * afterVertex() const;

    VertexCellSet vertices() const;

    // geometry
    Eigen::Vector2d pos(Time time) const;

    // serialization and copy
    void remapPointers(VAC * newVAC);
    friend QTextStream & ::operator<<(QTextStream & out, const AnimatedVertex & animatedVertex);
    friend QTextStream & ::operator>>(QTextStream & in, AnimatedVertex & animatedVertex);
    void convertTempIdsToPointers(VAC * vac);
    QString toString() const;
    void fromString(const QString & str);

    // Replace
    void replaceCells(InbetweenVertex * old, InbetweenVertex * new1, InbetweenVertex * new2);

    // Replace pointed vertex
    void replaceVertex(KeyVertex * oldVertex, KeyVertex * newVertex);

    // Direct access of inbetween vertices
    int size() const;
    InbetweenVertex * operator[] (int i) const;

private:
    InbetweenVertexList inbetweenVertices_;
    QList<int> tempIds_;
};

} // end namespace VectorAnimationComplex

#endif // ANIMATEDVERTEX_H
