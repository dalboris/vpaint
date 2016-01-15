// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef ZORDEREDCELLS_H
#define ZORDEREDCELLS_H

// ZOrderedCells: A doubly linked list of cells with convenient methods

#include "CellList.h"
#include "CellLinkedList.h"

namespace VectorAnimationComplex
{

class Cell;

class ZOrderedCells
{
public:
    ZOrderedCells();

    typedef CellLinkedList::Iterator Iterator;
    typedef CellLinkedList::ReverseIterator ReverseIterator;
    typedef CellLinkedList::ConstIterator ConstIterator;
    typedef CellLinkedList::ConstReverseIterator ConstReverseIterator;
    Iterator begin();
    Iterator end();
    ReverseIterator rbegin();
    ReverseIterator rend();
    ConstIterator cbegin() const;
    ConstIterator cend() const;
    ConstReverseIterator crbegin() const;
    ConstReverseIterator crend() const;

    void insertCell(Cell * cell); // insert just below boundary
    void insertLast(Cell * cell); // insert on top
    void removeCell(Cell * cell);
    void clear();

    Iterator find(Cell * cell);
    Iterator findFirst(const CellSet & cells);
    ReverseIterator findLast(const CellSet & cells);

    // Raise or lower a single cell

    void raise(Cell * cell);
    void lower(Cell * cell);
    void raiseToTop(Cell * cell);
    void lowerToBottom(Cell * cell);

    void altRaise(Cell * cell);
    void altLower(Cell * cell);
    void altRaiseToTop(Cell * cell);
    void altLowerToBottom(Cell * cell);

    // Raise or lower a set of cells

    void raise(CellSet cells);
    void lower(CellSet  cells);
    void raiseToTop(CellSet cells);
    void lowerToBottom(CellSet cells);

    void altRaise(CellSet cells);
    void altLower(CellSet cells);
    void altRaiseToTop(CellSet cells);
    void altLowerToBottom(CellSet cells);

    // Manually move cell c1 just below cell c2
    void moveBelow(Cell * c1, Cell * c2);
    void moveBelowBoundary(Cell * c);

private:
    CellLinkedList list_;

};

}

#endif // ZORDEREDCELLS_H
