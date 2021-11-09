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

#include "ZOrderedCells.h"

#include "Cell.h"
#include "Algorithms.h"

#include <iostream>
#include <QDebug>

namespace VectorAnimationComplex
{

ZOrderedCells::ZOrderedCells() :
    list_()
{
}

void ZOrderedCells::clear()
{
    list_.clear();
}

ZOrderedCells::Iterator ZOrderedCells::begin()
{
    return list_.begin();
}

ZOrderedCells::Iterator ZOrderedCells::end()
{
    return list_.end();
}

ZOrderedCells::ReverseIterator ZOrderedCells::rbegin()
{
    return list_.rbegin();
}

ZOrderedCells::ReverseIterator ZOrderedCells::rend()
{
    return list_.rend();
}

ZOrderedCells::ConstIterator ZOrderedCells::cbegin() const
{
    return list_.cbegin();
}

ZOrderedCells::ConstIterator ZOrderedCells::cend() const
{
    return list_.cend();
}

ZOrderedCells::ConstReverseIterator ZOrderedCells::crbegin() const
{
    return list_.crbegin();
}

ZOrderedCells::ConstReverseIterator ZOrderedCells::crend() const
{
    return list_.crend();
}

void ZOrderedCells::insertLast(Cell * cell)
{
    list_.append(cell);
}

// Insert the new cell just below the lowest boundary cell
void ZOrderedCells::insertCell(Cell * cell)
{
    // Get boundary cells
    CellSet boundary = cell->boundary();

    // Insert at appropriate position
    if(boundary.size() == 0)
    {
        // Insert last
        insertLast(cell);
    }
    else
    {
        // Insert before boundary
        Iterator it = begin();
        while(it!=end() && !boundary.contains(*it))
            ++it;
        list_.insert(it,cell);
    }
}

void ZOrderedCells::removeCell(Cell * cell)
{
    list_.remove(cell);
}

ZOrderedCells::Iterator ZOrderedCells::find(Cell * cell)
{
    Iterator it = begin();
    for(; it != end(); ++it)
        if(*it == cell)
            break;
    return it;
}

ZOrderedCells::Iterator ZOrderedCells::findFirst(const CellSet & cells)
{
    Iterator it = begin();
    for(; it != end(); ++it)
        if(cells.contains(*it))
            break;
    return it;
}

ZOrderedCells::ReverseIterator ZOrderedCells::findLast(const CellSet & cells)
{
    ReverseIterator it = rbegin();
    for(; it != rend(); ++it)
        if(cells.contains(*it))
            break;
    return it;
}

void ZOrderedCells::raise(Cell * cell) { raise(CellSet() << cell); }
void ZOrderedCells::lower(Cell * cell) { lower(CellSet() << cell); }
void ZOrderedCells::raiseToTop(Cell * cell) { raiseToTop(CellSet() << cell); }
void ZOrderedCells::lowerToBottom(Cell * cell) { lowerToBottom(CellSet() << cell); }

void ZOrderedCells::altRaise(Cell * cell) { altRaise(CellSet() << cell); }
void ZOrderedCells::altLower(Cell * cell) { altLower(CellSet() << cell); }
void ZOrderedCells::altRaiseToTop(Cell * cell) { altRaiseToTop(CellSet() << cell); }
void ZOrderedCells::altLowerToBottom(Cell * cell) { altLowerToBottom(CellSet() << cell); }

namespace // local free function
{

bool intersect(Cell * c, const CellSet & cells)
{
    for(Cell * c2: cells)
        if(c->boundingBox().intersects(c2->boundingBox()))
            return true;
    return false;
}

}

void ZOrderedCells::raise(CellSet cellsToRaise)
{
    int n = cellsToRaise.size();
    int nFound = 0;
    if(n == 0) return;

    // Find first cell to raise
    Iterator it = findFirst(cellsToRaise);
    if(it == end()) { qDebug() << "void ZOrderedCells::raise(Cell * cell): no cell found";    return;    }

    // List of actually raised cells (incrementally extracted from list_ in the loops that follow)
    CellLinkedList raisedCells;
    it = list_.extractTo(it,raisedCells);
    nFound++;

    // Get cells closure
    CellSet closure = Algorithms::closure(cellsToRaise);

    // First loop: advance it until we find c1 such that:
    //   - c1 is after every of the cells to raise (i.e., nFound == n)
    //   - c1 is not in the closure of the cells to raise
    //   - c1 intersects at least one cell to raise
    Cell * c1 = 0;
    while(it != end())
    {
        if(cellsToRaise.contains(*it))
        {
            it = list_.extractTo(it,raisedCells);
            nFound++;
        }
        else if(closure.contains(*it))
        {
            it = list_.extractTo(it,raisedCells);
        }
        else if( (nFound == n) && (intersect(*it,cellsToRaise)) )
        {
            c1 = *it;
            break;
        }
        else
            ++it;
    }
    if(!c1) // not found, raise to top.
    {
        list_.splice(it,raisedCells);
        return;
    }

    // Second loop: find the highest cell c2 such that:
    //   - c2 is not in the closure of the cells to raise
    //   - c2 is in the closure of c1
    CellSet c1Boundary = c1->boundary(); // Note: *it = c1 AND while(it2 != it) => no need for c1Closure
    Iterator it2 = end(); --it2;
    while(it2 != it)
    {
        if(c1Boundary.contains(*it2) && !closure.contains(*it2))
            break;
        else
            --it2;
    }

    // Third loop: finish to find cells to raise (i.e., boundary of c up to it2)
    while(it != it2)
    {
        if(closure.contains(*it))
            it = list_.extractTo(it,raisedCells);
        else
            ++it;
    }

    // Move raised cells above it2
    ++it2;
    list_.splice(it2,raisedCells);
}

void ZOrderedCells::lower(CellSet cellsToLower)
{
    int n = cellsToLower.size();
    int nFound = 0;
    if(n == 0) return;

    // Find first cell to lower
    ReverseIterator it = findLast(cellsToLower);
    if(it == rend()) { qDebug() << "void ZOrderedCells::lower(Cell * cell): no cell found";    return;    }

    // List of actually lowered cells (incrementally extracted from list_ in the loops that follow)
    CellLinkedList loweredCells;
    it = list_.extractTo(it,loweredCells);
    nFound++;

    // Get cells "fullstar" (i.e., star union itself)
    CellSet fullstar = Algorithms::fullstar(cellsToLower);

    // First loop: advance it until we find c1 such that:
    //   - c1 is before every of the cells to lower (i.e., nFound == n)
    //   - c1 is not in the fullstar of the cells to lower
    //   - c1 intersects at least one cell to lower
    Cell * c1 = 0;
    while(it != rend())
    {
        if(cellsToLower.contains(*it))
        {
            it = list_.extractTo(it,loweredCells);
            nFound++;
        }
        else if(fullstar.contains(*it))
        {
            it = list_.extractTo(it,loweredCells);
        }
        else if( (nFound == n) && (intersect(*it,cellsToLower)) )
        {
            c1 = *it;
            break;
        }
        else
            ++it;
    }
    if(!c1) // not found, raise to top.
    {
        list_.splice(it,loweredCells);
        return;
    }

    // Second loop: find the lowest cell c2 such that:
    //   - c2 is not in the fullstar of the cells to lower
    //   - c2 is in the closure of c1
    CellSet c1Star = c1->star(); // Note: *it = c1 AND while(it2 != it) => no need for c1Fullstar
    ReverseIterator it2 = rend(); --it2;
    while(it2 != it)
    {
        if(c1Star.contains(*it2) && !fullstar.contains(*it2))
            break;
        else
            --it2;
    }

    // Third loop: finish to find cells to lower (i.e., fullstar of c down to it2)
    while(it != it2)
    {
        if(fullstar.contains(*it))
            it = list_.extractTo(it,loweredCells);
        else
            ++it;
    }

    // Move lowered cells below it2
    ++it2;
    list_.splice(it2,loweredCells);
}

void ZOrderedCells::raiseToTop(CellSet cellsToRaise)
{
    // Return in trivial case
    int n = cellsToRaise.size();
    if(n == 0) return;

    // Get cells closure
    CellSet closure = Algorithms::closure(cellsToRaise);

    // List of actually raised cells
    CellLinkedList raisedCells;

    // Advance to top and extract closure
    Iterator it = begin();
    while(it != end())
    {
        if(closure.contains(*it))
            it = list_.extractTo(it,raisedCells);
        else
            ++it;
    }

    // Move raised cells to top
    list_.splice(it,raisedCells);
}

void ZOrderedCells::lowerToBottom(CellSet cellsToLower)
{
    // Return in trivial case
    int n = cellsToLower.size();
    if(n == 0) return;

    // Get cells fullstar
    CellSet fullstar = Algorithms::fullstar(cellsToLower);

    // List of actually lowered cells
    CellLinkedList loweredCells;

    // Advance to bottom and extract fullstar
    ReverseIterator it = rbegin();
    while(it != rend())
    {
        if(fullstar.contains(*it))
            it = list_.extractTo(it,loweredCells);
        else
            ++it;
    }

    // Move lowered cells to bottom
    list_.splice(it,loweredCells);
}

void ZOrderedCells::altRaise(CellSet cellsToRaise)
{
    int n = cellsToRaise.size();
    int nFound = 0;
    if(n == 0) return;

    // Find first cell to raise
    Iterator it = findFirst(cellsToRaise);
    if(it == end()) { qDebug() << "void ZOrderedCells::raise(Cell * cell): no cell found";    return;    }

    // List of actually raised cells
    CellLinkedList raisedCells;
    it = list_.extractTo(it,raisedCells);
    nFound++;

    // First loop: advance it until we find c1 such that:
    //   - c1 is after every of the cells to raise (i.e., nFound == n)
    //   - c1 intersects at least one cell to raise
    Cell * c1 = 0;
    while(it != end())
    {
        if(cellsToRaise.contains(*it))
        {
            it = list_.extractTo(it,raisedCells);
            nFound++;
        }
        else if( (nFound == n) && (intersect(*it,cellsToRaise)) )
        {
            c1 = *it;
            break;
        }
        else
            ++it;
    }
    if(!c1) // not found, raise to top.
    {
        list_.splice(it,raisedCells);
        return;
    }

    // Move raised cells above it
    ++it;
    list_.splice(it,raisedCells);
}

void ZOrderedCells::altLower(CellSet cellsToLower)
{
    int n = cellsToLower.size();
    int nFound = 0;
    if(n == 0) return;

    // Find first cell to lower
    ReverseIterator it = findLast(cellsToLower);
    if(it == rend()) { qDebug() << "void ZOrderedCells::lower(Cell * cell): no cell found";    return;    }

    // List of actually lowered cells
    CellLinkedList loweredCells;
    it = list_.extractTo(it,loweredCells);
    nFound++;

    // First loop: advance it until we find c1 such that:
    //   - c1 is before every of the cells to lower (i.e., nFound == n)
    //   - c1 intersects at least one cell to lower
    Cell * c1 = 0;
    while(it != rend())
    {
        if(cellsToLower.contains(*it))
        {
            it = list_.extractTo(it,loweredCells);
            nFound++;
        }
        else if( (nFound == n) && (intersect(*it,cellsToLower)) )
        {
            c1 = *it;
            break;
        }
        else
            ++it;
    }
    if(!c1) // not found, raise to top.
    {
        list_.splice(it,loweredCells);
        return;
    }

    // Move lowered cells below it
    ++it;
    list_.splice(it,loweredCells);
}

void ZOrderedCells::altRaiseToTop(CellSet cellsToRaise)
{
    // Return in trivial case
    int n = cellsToRaise.size();
    if(n == 0) return;

    // List of actually raised cells
    CellLinkedList raisedCells;

    // Advance to top and extract cells
    Iterator it = begin();
    while(it != end())
    {
        if(cellsToRaise.contains(*it))
            it = list_.extractTo(it,raisedCells);
        else
            ++it;
    }

    // Move raised cells to top
    list_.splice(it,raisedCells);
}

void ZOrderedCells::altLowerToBottom(CellSet cellsToLower)
{
    // Return in trivial case
    int n = cellsToLower.size();
    if(n == 0) return;

    // List of actually lowered cells
    CellLinkedList loweredCells;

    // Advance to bottom and extract cells
    ReverseIterator it = rbegin();
    while(it != rend())
    {
        if(cellsToLower.contains(*it))
            it = list_.extractTo(it,loweredCells);
        else
            ++it;
    }

    // Move lowered cells to bottom
    list_.splice(it,loweredCells);
}

void ZOrderedCells::moveBelow(Cell * c1, Cell * c2)
{
    Iterator it1 = find(c1);
    list_.erase(it1);

    Iterator it2 = find(c2);
    list_.insert(it2,c1);
}

void ZOrderedCells::moveBelowBoundary(Cell * c)
{
    CellSet boundary = c->boundary();
    if(!boundary.isEmpty())
    {
        Iterator it1 = find(c);
        list_.erase(it1);

        Iterator it2 = findFirst(boundary);
        list_.insert(it2,c);
    }
}

}
