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

CellLinkedList::ConstIterator CellLinkedList::cbegin() const
{
    return list_.cbegin();
}

CellLinkedList::ConstIterator CellLinkedList::cend() const
{
    return list_.cend();
}

CellLinkedList::ConstReverseIterator CellLinkedList::crbegin() const
{
    return list_.crbegin();
}

CellLinkedList::ConstReverseIterator CellLinkedList::crend() const
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
