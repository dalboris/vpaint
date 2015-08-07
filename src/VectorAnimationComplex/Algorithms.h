// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "CellList.h"

namespace VectorAnimationComplex
{

namespace Algorithms
{

// returns all the cells topologically connected to `cells` (super-set of cells)
CellSet connected(const CellSet & cells);

// decompose the set of edges in a list of connected components
// here, "connected" is in the sense that two edges are said "connected" if
// they share a common vertex.
QList<KeyEdgeSet> connectedComponents(const KeyEdgeSet & edges);

// returns the closure of a cell
CellSet closure(Cell * c);

// returns the closure of the set of cells
CellSet closure(const CellSet & cells);

// returns the full star (start union self) of a cell
CellSet fullstar(Cell * cell);

// returns the full star (start union self) of a set of cells
CellSet fullstar(const CellSet & cells);

// returns if two edges share a common vertex
// Special cases:
//   returns false if at least one of them is NULL
//   returns true if they are equals, even if it is a closed edge
bool areIncident(const KeyEdge * e1, const KeyEdge *e2);


} // namespace Algorithms

} // namespace VectorAnimationComplex

#endif // ALGORITHMS_H
