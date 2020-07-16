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
