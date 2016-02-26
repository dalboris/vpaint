// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef IDMANAGER_H
#define IDMANAGER_H

#include <map>
#include <vector>
#include <cassert>

namespace OpenVAC
{

/// \class IdManager Core/IdManager.h
/// \brief IdManager is a container that assigns unique IDs to stored elements.

template <class T>
class IdManager
{
public:
    typedef unsigned int Id;

    IdManager() : map_() {}

    /// Inserts the given element and assigns it a unique ID.
    /// Returns the ID assigned to the element.
    Id insert(const T & value)
    {
        Id id = getAvailableId();
        map_[id] = value;
        return id;
    }

    /// Inserts the given element with the given ID.
    /// Aborts if the given ID was already assigned.
    void insert(Id id, const T & value)
    {
        assert(isAvailable(id));
        map_[id] = value;
    }

    /// Returns false if no element is assigned the given ID; otherwise
    /// removes the element with the given ID and returns true.
    bool remove(Id id)
    {
        return map_.erase(id);
    }

    /// Returns whether the manager contains an element with the given ID.
    ///
    bool contains(Id id) const
    {
        return map_.find(id) != map_.end();
    }

    /// Returns whether the given ID is available (i.e., not yet assigned to
    /// any element).
    bool isAvailable(Id id) const
    {
        return !contains(id);
    }

    /// Returns an available ID.
    ///
    Id getAvailableId() const
    {
        return maxId_() + 1;
    }

    /// Returns numIds available IDs.
    ///
    std::vector<Id> getAvailableIds(unsigned int numIds) const
    {
        Id m = maxId_() + 1;
        std::vector<Id> res(numIds);
        for (unsigned int i = 0; i < numIds; ++i)
        {
            res[i] =  m + i;
        }
        return res;
    }

    /// Returns a reference to the element with given ID. If no such element
    /// exists, an exception of type std::out_of_range is thrown.
    T & operator[](Id id) { return map_.at(id); }
    const T & operator[](Id id) const { return map_.at(id); }

private:
    std::map<Id, T> map_;

    Id maxId_() const
    {
        return map_.empty() ? 0 : map_.crbegin()->first;
    }
};

}

#endif // IDMANAGER_H
