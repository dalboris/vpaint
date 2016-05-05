// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_IDMANAGER_H
#define OPENVAC_IDMANAGER_H

#include <map>
#include <vector>
#include <cassert>

namespace OpenVac
{

/// \class IdManager OpenVac/Core/IdManager.h
/// \brief A container that assigns unique IDs to stored elements.
///
template <class T>
class IdManager
{
public:
    using Id       = unsigned int;
    using map_type = std::map<Id, T>;
    using iterator = typename map_type::iterator;
    using const_iterator = typename map_type::const_iterator;

    /// Constructs an IdManager.
    ///
    IdManager() : map_() {}

    /// Inserts the given element and assigns it a unique ID.
    /// Returns the ID assigned to the element.
    ///
    Id insert(const T & value)
    {
        Id id = getAvailableId();
        map_[id] = value;
        return id;
    }

    /// Inserts the given element with the given ID.
    /// Aborts if the given ID was already assigned.
    ///
    ///
    void insert(Id id, const T & value)
    {
        assert(isAvailable(id));
        map_[id] = value;
    }

    /// Returns false if no element is assigned the given ID; otherwise
    /// removes the element with the given ID and returns true.
    ///
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
    ///
    bool isAvailable(Id id) const
    {
        return !contains(id);
    }

    /// Returns an available ID.
    ///
    Id getAvailableId() const
    {
        // XXX instead, test if maxId_ > k * size (e.g., k=2),
        // and if it's the case, first take a random Id in [0, maxId-1]
        // and test availability. If available then return it, otherwise return
        // maxId_ + 1;
        return maxId_() + 1;
    }

    // XXX Id getAvailableId(const std::vector<Id> & forbiddenIds) { ... }

    /// Returns numIds available IDs.
    ///
    std::vector<Id> getAvailableIds(unsigned int numIds) const
    {
        // XXX same comment as in getAvailableId()

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
    ///
    T & operator[](Id id)
    {
        return map_.at(id);
    }

    /// Returns a const reference to the element with given ID. If no such
    /// element exists, an exception of type std::out_of_range is thrown.
    ///
    const T & operator[](Id id) const
    {
        return map_.at(id);
    }

    /// Returns the number of elements in the container
    ///
    size_t size() const { return map_.size(); }

    /// Returns an iterator that points to the first pair in the Id manager.
    ///
    iterator begin()
    {
        return map_.begin();
    }

    /// Returns a read-only (constant) iterator that points to the first
    /// pair in the Id manager.
    ///
    const_iterator begin() const
    {
        return map_.begin();
    }

    /// Returns an iterator that points one past the last pair in the Id
    /// manager.
    ///
    iterator end()
    {
        return map_.end();
    }

    /// Returns a read-only (constant) iterator that points one past the last
    /// pair in the Id manager.
    ///
    const_iterator end() const
    {
        return map_.end();
    }

    /// Tries to locate an element with the given ID. Returns an iterator
    /// pointing to sought-after element, or end() if not found.
    ///
    iterator find(Id id)
    {
        return map_.find(id);
    }

    /// Tries to locate an element with the given ID. Returns a read-only
    /// (constant) iterator pointing to sought-after element, or end() if not
    /// found.
    ///
    const_iterator find(Id id) const
    {
        return map_.find(id);
    }

private:
    std::map<Id, T> map_; // XXX Use unordered_map instead?

    Id maxId_() const
    {
        // XXX if use unordered_map, then maxId_ must be tracked manually
        return map_.empty() ? 0 : map_.crbegin()->first;
    }
};

} // end namespace OpenVac

#endif // OPENVAC_IDMANAGER_H
