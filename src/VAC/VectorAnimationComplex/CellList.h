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

#ifndef VAC_CELL_LIST_H
#define VAC_CELL_LIST_H

#include <QList>
#include <QSet>
#include "ForwardDeclaration.h"

// In this file, we define the following classes:
//  - CellList (inheriting QList<Cell*>)
//  - VertexCellList (inheriting QList<VertexCell*>)
//  [...]
//  - CellSet (inheriting QSet<Cell*>)
//  - VertexCellSet (inheriting QSet<VertexCell*>)
//  [...]
//
// The whole point of these classes and their convoluted macro/template definition
// is to make it very convenient to convert between these containers. Example usage:
//   CellList cellList = cells();
//   KeyVertexSet keyVertexSet = cellList;
//
// This gets you the set of all key vertices which belongs to the list of all cells.
// The cells in cellList which are not key vertices are safely ignored and not inserted
// into keyVertexSet.

namespace VectorAnimationComplex
{

// Templated method to convert, for instance, a QList<Cell*> into a QSet<KeyVertex*>
//
// Example:
//   QList<Cell*>     cellList;
//   QSet<KeyVertex*> keyVertexSet;
//   copyCellContainer<Cell,QList<Cell*>,KeyVertex,Set<KeyVertex*>>(cellList,keyVertexSet);
//
// For each Cell* in cellList, the method performs a dynamic cast to check whether the Cell*
// can be casted to a KeyVertex*, and if yes, insert it into keyVertexSet

template<class U, class UContainer, class T, class TContainer>
void copyCellContainer(const UContainer & from, TContainer & to)
{
    to.clear();
    for(U * u: from)
    {
        if(u)
        {
            T * t = u->template to<T>();
            if(t) to << t;
        }
    }
}

// Below are three preprocessor macros :
//   * The first two macros are simply helpers to help define the third macro
//   * CELLCONTAINER_PTRCONTAINER(QList,KeyVertex) defines a class called
//     KeyVertexPtrQList which inherits from QList<KeyVertex*> and can be
//     assigned from any QList<U*> or QSet<T*>

#define CELLCONTAINER_COPYCONSTRUCTOR(ThisContainer,T,OtherContainer) \
    template<class U> ThisContainer(const OtherContainer<U*> & other) { \
        copyCellContainer<U,OtherContainer<U*>,T,ThisContainer>(other, *this); }

#define CELLCONTAINER_ASSIGNMENTOPERATOR(ThisContainer,T,OtherContainer) \
    template<class U> ThisContainer & operator=(const OtherContainer<U*> & other) { \
        if (this != &other) \
            copyCellContainer<U,OtherContainer<U*>,T,ThisContainer>(other, *this); \
        return *this; }

#define CELLCONTAINER_PTRCONTAINER(BaseContainer,T) \
    class T##Ptr##BaseContainer: public BaseContainer<T*> \
    { \
    public: \
        T##Ptr##BaseContainer() {} \
        \
        CELLCONTAINER_COPYCONSTRUCTOR(T##Ptr##BaseContainer,T,QList) \
        CELLCONTAINER_ASSIGNMENTOPERATOR(T##Ptr##BaseContainer,T,QList) \
        \
        CELLCONTAINER_COPYCONSTRUCTOR(T##Ptr##BaseContainer,T,QSet) \
        CELLCONTAINER_ASSIGNMENTOPERATOR(T##Ptr##BaseContainer,T,QSet) \
    };

// Below is the actual definition of the convenient classes
// Example:
//   KeyVertexList is a typedef for KeyVertexPtrQList, which inherits from QList<KeyVertex*>,
//   and can be copy constructed or assigned from any QList<T*> or QSet<T*>, whenever T inherits
//   from Cell. By extension, it can also be copy constructed or assigned from any class which
//   inherits from QList<T*> or QSet<T*>, for instance the classes defined below.

#define CELLCONTAINER_DEFINE(T,ContainerType)\
    CELLCONTAINER_PTRCONTAINER(Q##ContainerType,T)\
    typedef T##Ptr##Q##ContainerType T##ContainerType;

CELLCONTAINER_DEFINE(Cell,List)
CELLCONTAINER_DEFINE(VertexCell,List)
CELLCONTAINER_DEFINE(EdgeCell,List)
CELLCONTAINER_DEFINE(FaceCell,List)
CELLCONTAINER_DEFINE(KeyCell,List)
CELLCONTAINER_DEFINE(InbetweenCell,List)
CELLCONTAINER_DEFINE(KeyVertex,List)
CELLCONTAINER_DEFINE(KeyEdge,List)
CELLCONTAINER_DEFINE(KeyFace,List)
CELLCONTAINER_DEFINE(InbetweenVertex,List)
CELLCONTAINER_DEFINE(InbetweenEdge,List)
CELLCONTAINER_DEFINE(InbetweenFace,List)

CELLCONTAINER_DEFINE(Cell,Set)
CELLCONTAINER_DEFINE(VertexCell,Set)
CELLCONTAINER_DEFINE(EdgeCell,Set)
CELLCONTAINER_DEFINE(FaceCell,Set)
CELLCONTAINER_DEFINE(KeyCell,Set)
CELLCONTAINER_DEFINE(InbetweenCell,Set)
CELLCONTAINER_DEFINE(KeyVertex,Set)
CELLCONTAINER_DEFINE(KeyEdge,Set)
CELLCONTAINER_DEFINE(KeyFace,Set)
CELLCONTAINER_DEFINE(InbetweenVertex,Set)
CELLCONTAINER_DEFINE(InbetweenEdge,Set)
CELLCONTAINER_DEFINE(InbetweenFace,Set)

}

#endif
