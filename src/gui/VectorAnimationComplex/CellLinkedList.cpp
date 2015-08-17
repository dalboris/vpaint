// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "CellLinkedList.h"

namespace VectorAnimationComplex
{

CellLinkedList::CellLinkedList()
{
}

CellLinkedList::Iterator CellLinkedList::begin()
{
    return list_.begin();
}

CellLinkedList::Iterator CellLinkedList::end()
{
    return list_.end();
}

CellLinkedList::ReverseIterator CellLinkedList::rbegin()
{
    return list_.rbegin();
}

CellLinkedList::ReverseIterator CellLinkedList::rend()
{
    return list_.rend();
}

CellLinkedList::ConstIterator CellLinkedList::cbegin()
{
    return list_.cbegin();
}

CellLinkedList::ConstIterator CellLinkedList::cend()
{
    return list_.cend();
}

CellLinkedList::ConstReverseIterator CellLinkedList::crbegin()
{
    return list_.crbegin();
}

CellLinkedList::ConstReverseIterator CellLinkedList::crend()
{
    return list_.crend();
}

void CellLinkedList::clear()
{
    list_.clear();
}

void CellLinkedList::append(Cell * cell)
{
    list_.push_back(cell);
}

void CellLinkedList::prepend(Cell * cell)
{
    list_.push_front(cell);
}

void CellLinkedList::remove(Cell * cell)
{
    list_.remove(cell);
}

CellLinkedList::Iterator CellLinkedList::insert(CellLinkedList::Iterator pos, Cell * cell)
{
    return list_.insert(pos,cell);
}

CellLinkedList::Iterator CellLinkedList::erase(CellLinkedList::Iterator pos)
{
    return list_.erase(pos);
}

void CellLinkedList::splice( CellLinkedList::Iterator pos, CellLinkedList & other )
{
    list_.splice(pos, other.list_);
}

CellLinkedList::Iterator CellLinkedList::extractTo(CellLinkedList::Iterator pos, CellLinkedList & other)
{
    Cell * c = *pos;
    other.append(c);
    return erase(pos);
}

// Reverse methods

CellLinkedList::ReverseIterator CellLinkedList::insert(CellLinkedList::ReverseIterator pos, Cell * cell)
{
    Iterator it = insert(pos.base(), cell);
    return ReverseIterator(++it);
}

CellLinkedList::ReverseIterator CellLinkedList::erase(CellLinkedList::ReverseIterator pos)
{
    Iterator it = (++pos).base();
    it = erase(it);
    return ReverseIterator(it);
}

void CellLinkedList::splice(CellLinkedList::ReverseIterator pos, CellLinkedList & other )
{
    splice(pos.base(), other);
}

CellLinkedList::ReverseIterator CellLinkedList::extractTo(CellLinkedList::ReverseIterator pos, CellLinkedList & other)
{
    Cell * c = *pos;
    other.prepend(c);
    return erase(pos);
}

}
