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

#ifndef CELLLINKEDLIST_H
#define CELLLINKEDLIST_H

#include <list>

namespace VectorAnimationComplex
{

class Cell;
class CellLinkedList
{
public:
    CellLinkedList();

    typedef std::list<Cell*>::iterator Iterator;
    typedef std::list<Cell*>::const_iterator ConstIterator;
    typedef std::list<Cell*>::reverse_iterator ReverseIterator;
    typedef std::list<Cell*>::const_reverse_iterator ConstReverseIterator;
    Iterator begin();
    Iterator end();
    ReverseIterator rbegin();
    ReverseIterator rend();
    ConstIterator cbegin() const;
    ConstIterator cend() const;
    ConstReverseIterator crbegin() const;
    ConstReverseIterator crend() const;

    void clear();
    void append(Cell * cell);
    void prepend(Cell * cell);
    void remove(Cell * cell);

    Iterator insert(Iterator pos, Cell * cell);
    Iterator erase(Iterator pos);
    void splice(Iterator pos, CellLinkedList & other );
    Iterator extractTo(Iterator pos, CellLinkedList & other); // append *pos to other, then return erase(pos)

    // Same in reverse
    ReverseIterator insert(ReverseIterator pos, Cell * cell);
    ReverseIterator erase(ReverseIterator pos);
    void splice(ReverseIterator pos, CellLinkedList & other ); // note: since other is inserted between *pos and *(pos.base()),
                                                               //       then after calling this method, *pos becomes the last
                                                               //       element of other. This is a difference of semantics with
                                                               //       splice(Iterator pos), which does not affect which element
                                                               //       pos points to
    ReverseIterator extractTo(ReverseIterator pos, CellLinkedList & other); // prepend *pos to other, then return erase(pos)

private:
    std::list<Cell*> list_;
};

}

#endif // CELLLINKEDLIST_H
