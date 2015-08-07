// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

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
    ConstIterator cbegin();
    ConstIterator cend();
    ReverseIterator rbegin();
    ReverseIterator rend();
    ConstReverseIterator crbegin();
    ConstReverseIterator crend();

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
