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
#include <cassert>

namespace OpenVac
{

// Shared pointer
template <class T>
using SharedPtr = std::shared_ptr<T>;

// Weak pointer. Like std::weak_ptr but providing the additional:
//   - operator bool(): Returns true if use_count() > 0 && stored pointer non null
//   - get(), operator*, and operator->: Returns stored pointer. Aborts if use_count() == 0.
//     doesn't require a lock(): the stored pointer is cached. You can still perform a
//     lock yourself if thread safety is an issue in your case.
template <class T>
class WeakPtr
{
public:
    typedef T element_type;

    // Constructors
    WeakPtr() :
        wp_(),
        p_(nullptr)
    {
    }

    WeakPtr(const WeakPtr & r) :
        wp_(r.wp_),
        p_(r.p_)
    {
    }

    template <class Y>
    WeakPtr(const WeakPtr<Y> & r) :
        wp_(r.wp_),
        p_(r.p_)
    {
    }

    template <class Y>
    WeakPtr(const SharedPtr<Y> & r) :
        wp_(r),
        p_(r.get()) // Warning: r.use_count() == 0 does not guarantee r.get() == nullptr
    {
    }

    // Destructor
    ~WeakPtr() {}

    // Assignment operators
    WeakPtr & operator=(const WeakPtr & r)
    {
        wp_ = r.wp_;
        p_ = r.p_;
        return *this;
    }

    template <class Y>
    WeakPtr & operator=(const WeakPtr<Y> & r)
    {
        wp_ = r.wp_;
        p_ = r.p_;
        return *this;
    }

    template <class Y>
    WeakPtr & operator=(const SharedPtr<Y> & r)
    {
        wp_ = r;
        p_ = r.get();
        return *this;
    }

    // Reset
    void reset()
    {
        wp_.reset();
        p_ = nullptr;
    }

    // Swap
    void swap(WeakPtr & r)
    {
        wp_.swap(r.wp_);
        swap(p_, r.p_);
    }

    // Use count
    long use_count() const { return wp_.use_count(); }

    // Expired
    bool expired() const { return wp_.expired(); }

    // Lock
    SharedPtr<T> lock() const { return wp_.lock(); }

    // Owner before
    template <class Y>
    bool owner_before(const WeakPtr<Y> & other) const { return wp_.owner_before(other.wp_); }

    template <class Y>
    bool owner_before(const SharedPtr<Y> & other) const { return wp_.owner_before(other); }

    // Conversion to bool
    operator bool() const { return use_count() > 0 && p_; }

    // Dereferencing
    T * get() const { assert(use_count() > 0); return p_; }
    T & operator*() const { return *get(); }
    T * operator->() const { return get(); }

    // Comparison with other smart pointer types
    template <class YPtr>
    bool operator==(const YPtr & otherPtr)
    {
        bool valid = use_count();
        bool otherValid = otherPtr.use_count();

        if (valid && otherValid)
            return get() == otherPtr.get(); // Both valid with same pointer value
        else
            return valid == otherValid;     // Both invalid
    }

private:
    std::weak_ptr<T> wp_;
    T * p_;
};

template <class T>
void swap(WeakPtr<T> & lhs, WeakPtr<T> & rhs )
{
    return lhs.swap(rhs);
}

template<class T>
class WeakPtrProvider {
public:
    WeakPtrProvider(T * t) : fakeShared_(t, dontDelete), weakToShared_(fakeShared_)
    {}

    WeakPtr<T> & ptr()
    {
        return weakToShared_;
    }

    // Sets the shared pointer that provided weak pointers will refer too.
    // Causes undefined behaviour if ptr() has been called before setShared().
    // In other words, there are only two usage scenarios:
    //
    //   1) You never call setShared (when t is stack-allocated, member attribute,
    //      or managed by unique_ptr)
    //
    //   2) You call setShared() first and once only, followed by any numbers
    //      of calls to ptr() (when t is stack-allocated, member attribute, or
    //      managed by unique_ptr)
    void setShared(const SharedPtr<T> & sp)
    {
        fakeShared_.reset();
        weakToShared_ = sp;
    }

private:
    // Fake shared pointer to be able to provide a weak pointer to an object
    // which is not managed via shared_ptr (e.g., stack-allocated, or member
    // attribute, or object managed via unique_ptr), i.e. when setShared() is
    // not called. In this case, the lifetime of the weak_ptr will be the
    // same as the lifetime of this WeakPtrProvider.
    SharedPtr<T> fakeShared_;
    static void dontDelete(T*) {}

    // Weak pointer to a client-managed shared pointer (which has been
    // indicated by setShared()), or to the fake shared pointer.
    WeakPtr<T> weakToShared_;

};

}

#endif // OPENVAC_MEMORY_H
