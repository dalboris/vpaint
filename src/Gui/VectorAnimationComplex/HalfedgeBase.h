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

/* -------------------- Halfedge.h --------------------
 *
 * In a nutshell, we want to define the following three classes:
 *
 *    struct Halfedge          { EdgeCell *      edge;  bool side; };
 *    struct KeyHalfedge       { KeyEdge *       edge;  bool side; };
 *    struct InbetweenHalfedge { InbetweenEdge * edge;  bool side; };
 *
 * In practice, to factorize code, we define a templated class:
 *
 *    template <class TEdge> struct HalfedgeBase { TEdge * edge;  bool side; [+ methods common to all 3 classes] };
 *
 * Which is used to define our three classes:
 *
 * typedef HalfedgeBase<EdgeCell> Halfedge;
 * class KeyHalfedge:       public HalfedgeBase<KeyEdge>       { [methods specific to KeyHalfedge] };
 * class InbetweenHalfedge: public HalfedgeBase<InbetweenEdge> { [methods specific to InbetweenHalfedge] };
 *
 */

#ifndef VAC_HALFEDGEBASE_H
#define VAC_HALFEDGEBASE_H

#include <QTextStream>

#include "ForwardDeclaration.h"
#include "EdgeCell.h"

namespace VectorAnimationComplex
{

// ----------------------------- HalfedgeImpl ---------------------------------
//      (free functions to move most implementation to Halfedge.cpp)

namespace HalfedgeImpl
{

Cell * cellFromId(VAC * vac, int id);
Cell * cellFromId(VAC * vac, Cell * cell);

template <class TEdge> TEdge * edgeFromCell(Cell * c) { return ( c ? c->to<TEdge>() : 0 ); }
template <class TEdge> TEdge * edgeFromId(VAC * vac, int id) { return edgeFromCell<TEdge>(cellFromId(vac, id)); }
template <class TEdge> TEdge * remapedEdge(VAC * vac, TEdge * edge) { return edgeFromCell<TEdge>(cellFromId(vac, edge)); }

void save(EdgeCell * edge, bool side, QTextStream & out);

bool isEqual(EdgeCell * edge1, bool side1, EdgeCell * edge2, bool side2);
bool isValid(EdgeCell * edge);
bool isClosed(EdgeCell * edge);

EdgeSample startSample(EdgeCell * edge, bool side, Time t);
EdgeSample endSample(EdgeCell * edge, bool side, Time t);

}



// -------------------------------- HalfedgeBase ------------------------------

template <class TEdge>
class HalfedgeBase
{
public:
    // Data members
    TEdge * edge;
    bool side;

    // Constructors
    HalfedgeBase() : edge(0), side(true) {}
    HalfedgeBase(TEdge * e, bool s) : edge(e), side(s) {}

    // Operators
    bool operator==(const HalfedgeBase & other) const { return HalfedgeImpl::isEqual(edge,side,other.edge,other.side); }

    // Cloning
    void remapPointers(VAC * vac) { edge = HalfedgeImpl::remapedEdge<TEdge>(vac, edge); }

    // I/O
    int tempId_;
    void save(QTextStream & out) const    { HalfedgeImpl::save(edge, side, out); }
    void convertTempIdsToPointers(VAC * vac) { edge = HalfedgeImpl::edgeFromId<TEdge>(vac, tempId_); }

    // Other methods
    bool isValid() const { return HalfedgeImpl::isValid(edge); }
    bool isClosed() const { return HalfedgeImpl::isClosed(edge); }
    EdgeSample startSample(Time t) { return HalfedgeImpl::startSample(edge, side, t); }
    EdgeSample endSample(Time t) {  return HalfedgeImpl::endSample(edge, side, t); }


    // Navigation
    HalfedgeBase opposite() {return HalfedgeBase(edge, !side);}
};



// ------------ Hash function to allow creating sets of halfedges -------------

template <class TEdge> uint qHash(const HalfedgeBase<TEdge> & h) { return ::qHash(h.edge) ^ ::qHash(h.side); }



// ---- Generic halfedge, can point either to a KeyEdge or a InbetweenEdge ----

typedef HalfedgeBase<EdgeCell> Halfedge;


} // end namespace VectorAnimationComplex



// ----------------------------------- I/O ------------------------------------

template <class TEdge>
QTextStream & operator<<(QTextStream & out, const VectorAnimationComplex::HalfedgeBase<TEdge> & h)
{
    h.save(out);
    return out;
}

template <class TEdge>
QTextStream & operator>>(QTextStream & in, VectorAnimationComplex::HalfedgeBase<TEdge> & h)
{
    QString nuple; in >> nuple;
    QStringList list = nuple.split(QRegExp("\\s*[\\(\\,\\)]\\s*"), QString::SkipEmptyParts);
    h.edge = 0;
    h.tempId_ = list[0].toInt();
    h.side = list[1].toInt();
    return in;
}

#endif // VAC_HALFEDGEBASE_H
