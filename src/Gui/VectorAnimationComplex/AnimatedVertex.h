// Copyright (C) 2012-2019 The VPaint Developers
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
