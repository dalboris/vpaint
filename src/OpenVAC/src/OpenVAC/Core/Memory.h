// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_MEMORY_H
#define OPENVAC_MEMORY_H

#include <memory>

namespace OpenVAC
{

// Shared pointer
template <class T>
using SharedPtr = std::shared_ptr<T>;

// Weak pointer. Like std::weak_ptr but providing the additional functionality:
//   - operator bool(): returns true if it's not empty, not expired, and the
//                      stored pointer is not null
//   - get(), operator*, and operator->: returns null if the weak pointer is
//                      empty or expired, otherwise return the store pointer
template <class T>
class WeakPtr
{
public:
    // Constructors
    WeakPtr() : ptr_() {}

    WeakPtr(const WeakPtr & r) : ptr_(r.ptr_) {}

    template <class Y>
    WeakPtr(const WeakPtr<Y> & r) : ptr_(r.ptr_) {}

    template <class Y>
    WeakPtr(const SharedPtr<Y> & r) : ptr_(r) {}

    // Destructor
    ~WeakPtr() {}

    // Assignment operators
    WeakPtr & operator=(const WeakPtr & r) { ptr_ = r.ptr_; return *this; }

    template <class Y>
    WeakPtr & operator=(const WeakPtr<Y> & r) { ptr_ = r.ptr_; return *this; }

    template <class Y>
    WeakPtr & operator=(const SharedPtr<Y> & r) { ptr_ = r; return *this; }

    // Reset
    void reset() { ptr_.reset(); }

    // Swap
    void swap( WeakPtr & r ) { ptr_.swap(r.ptr_); }

    // Use count
    long use_count() const { return ptr_.use_count(); }

    // Expired
    bool expired() const { return ptr_.expired(); }

    // Lock
    SharedPtr<T> lock() const { return ptr_.lock(); }

    // Owner before
    template <class Y>
    bool owner_before(const WeakPtr<Y> & other) const { return ptr_.owner_before(other); }

    template <class Y>
    bool owner_before(const SharedPtr<Y> & other) const { return ptr_.owner_before(other); }

    // Conversion to bool
    operator bool() const { return use_count() > 0 && (bool)lock(); }

    // Dereferencing
    T * get() const { return use_count() > 0 ? lock().get() : nullptr; }
    T & operator*() const { return *get(); }
    T * operator->() const { return get(); }

private:
    std::weak_ptr<T> ptr_;
};

template <class T>
void swap(WeakPtr<T> & lhs, WeakPtr<T> & rhs )
{
    return lhs.swap(rhs);
}

}

#define OPENVAC_FORWARD_DECLARE_SHARED_PTR(T) \
    class T; \
    typedef OpenVAC::SharedPtr<T> T##SharedPtr;

#define OPENVAC_FORWARD_DECLARE_PTR(T) \
    class T; \
    typedef OpenVAC::WeakPtr<T> T##Ptr;

#define OPENVAC_DECLARE_SHARED_PTR(T) \
    typedef OpenVAC::SharedPtr<T> T##SharedPtr;

#define OPENVAC_DECLARE_PTR(T) \
    typedef OpenVAC::WeakPtr<T> T##Ptr;

#endif // OPENVAC_MEMORY_H
