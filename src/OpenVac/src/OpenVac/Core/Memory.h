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
/// \page memorymodel Memory Model
///
/// A page about the memory model used in OpenVac. Will explain
/// the semantics of SharedPtr, WeakPtr, and Handle.


/******************************** SharedPtr **********************************/

/// \typedef OpenVac::SharedPtr OpenVac/Core/Geometry.h
/// \brief Alias for std::shared_ptr

template <class T>
using SharedPtr = std::shared_ptr<T>;


/******************************** WeakPtr **********************************/

/// \class WeakPtr OpenVac/Core/Geometry.h
/// \brief Extension of std::weak_ptr
///
/// Extends std::weak_ptr by providing the additional:
///   - operator bool(): Returns true if both use_count() > 0 and the stored
///     pointer is non null
///   - get(), operator*, and operator->: Returns the stored pointer. Aborts if
///     use_count() == 0. This does not instantiate a shared pointer via lock().
///     Therefore, this shouldn't be used if the pointed object may be deleted
///     in a concurrent thread.
///   - operator==. As with dereferencing, this shouldn't be used if the pointed
///     object may be deleted in a concurrent thread.
///
/// <H2>Example</H2>
///
/// \code
/// // Example usage when the pointer is not supposed to be expired or
/// // null, by design. This is the recommended usage. Dereferencing
/// // is guaranteed to crash your application if the pointer is expired or
/// // null (i.e., if the code or design is broken somewhere else). This is a
/// // much better behaviour than the "undefined behaviour" of raw pointer, and
/// // more readable and maintainable than writing the (almost) equivalent:
/// //
/// //     if (auto sp = wp.lock()) { ... } else { abort(); }
/// //
///
/// void assumesNotExpiredAndWontExpire(const WeakPtr<Foo> & foo)
/// {
///     foo->doSomething();
///     foo->doSomethingElse();
/// }
///
/// // Example usage when the pointer may be expired or null. Though,
/// // whenever possible, try to use a notification system in order to know
/// // when objects are expiring and not call functions that depend on them
/// // in the first place. This way, you don't have to write such "smart"
/// // functions that make the code less readable ("smart" = gracefully deal
/// // with expired case).
///
/// void doesntAssumeAnything(const WeakPtr<Foo> & foo)
/// {
///     if (auto sfoo = foo.lock())
///     {
///         sfoo->doSomething();
///         sfoo->doSomethingElse();
///     }
///     else
///     {
///         handleExpiredCase();
///     }
/// }
///
/// // Example usage when the pointer may be expired or null, but when this
/// // state is not supposed to change concurrently during the execution of
/// // the function. This is the least recommended usage. If you can't assume
/// // the pointer may be expired or null, you'd better not assume it can't
/// // change concurrently as well, as this would prevent later parallelization,
/// // and it doesn't save you much typing anyway.
///
/// void assumesIsExpiredOrWontExpire(const WeakPtr<Foo> & foo)
/// {
///     if (foo)
///     {
///         foo->doSomething();
///         foo->doSomethingElse();
///     }
///     else
///     {
///         handleExpiredCase();
///     }
/// }
/// \endcode

template <class T>
class WeakPtr
{
public:
    using element_type = T;

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
        p_(r.get()) // Note: r.use_count() == 0 does not guarantee r.get() == nullptr
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
    bool operator==(const YPtr & other)
    {
        bool valid = use_count();
        bool otherValid = other.use_count();

        if (valid && otherValid)
            return get() == other.get(); // Both valid with same pointer value
        else
            return valid == otherValid;  // Both invalid
    }

private:
    std::weak_ptr<T> wp_;
    T * p_;
};

template <class T>
void swap(WeakPtr<T> & lhs, WeakPtr<T> & rhs)
{
    return lhs.swap(rhs);
}


/************************** enable_ptrs_from_this ****************************/
/*
/// \class enable_ptrs_from_this OpenVac/Core/Memory.h
/// \brief Allows to safely generate weak and shared pointers from objects.
///
/// Provides the functions make_shared(), shared_from_this(), and
/// weak_from_this().
///
/// The static member function make_shared() returns a shared pointer managing
/// a new instance of T. Instances of T managed by shared pointers *must* be
/// allocated through this function. Using directly std::make_shared, the
/// constructors of std::shared_ptr, or the constructors of OpenVac::SharedPtr
/// with classes inheriting enable_ptrs_from_this is undefined behavior.
///
/// The member function shared_from_this() returns a shared pointer that shares
/// ownership of \p this with existing shared pointers managing \p this. Aborts
/// if \p this was not allocated via the static member function make_shared().
///
/// The member function weak_from_this() returns a weak pointer to \p this, who
/// may have been allocated using the static member function make_shared(),
/// or is stack-allocated, or is a member variable, or is managed via a
/// std::unique_ptr. Though, in all but the first case, calling lock() on
/// the returned weak pointer won't extend the lifetime of the object. Though,
/// use_count() and operator== behave as expected and correctly assess whether
/// the destructor of the object has already been called or not. Therefore,
/// you just need to make sure that the object cannot be deleted in a concurrent
/// thread after testing that it wasn't expired.
///
template <class T>
class enable_ptrs_from_this
{
public:
    enable_ptrs_from_this() :
        fakeShared_(static_cast<T*>(*this), dontDelete_),
        weakToShared_(fakeShared_),
        isShared_(false)
    {}

    /// Generates a weak pointer to this object.
    const WeakPtr<T> & weak_from_this()
    {
        return weakToShared_;
    }

    /// Generates an additional shared pointer that shares ownership of \p this
    /// with existing shared pointers managing \p this. Aborts if \p this was
    /// not created via enable_ptrs_from_this::make_shared
    ///
    /// with
    ///  weak pointer to this object
    SharedPtr<T> shared_from_this()
    {
        assert(isShared_);
        return weakToShared_.lock();
    }

    template <typename... Args>
    static SharedPtr<T> make_shared(Args&&... args)
    {
        SharedPtr<T> res = std::make_shared<T>(std::forward<Args>(args)...);
        auto * me = static_cast<enable_ptrs_from_this*>(res.get());
        me->weakToShared_ = res;
        me->isShared_ = true;
        return res;
    }

private:
    // Fake shared pointer to be able to provide a weak pointer to an object
    // which is not managed via shared_ptr (e.g., stack-allocated, or member
    // attribute, or object managed via unique_ptr), i.e. when setShared() is
    // not called. In this case, the lifetime of the weak_ptr will be the
    // same as the lifetime of this WeakPtrProvider.
    SharedPtr<T> fakeShared_;
    static void dontDelete_(T*) {}

    // Weak pointer to a client-managed shared pointer (which has been
    // indicated by setShared()), or to the fake shared pointer.
    WeakPtr<T> weakToShared_;

    // Stores whether thjis object is managed by a shared pointer or not
    bool isShared_;
};
*/

/*********************************** Handle **********************************/

// Forward declaration of Handle class template
template <class T> class Handle;

/// Converts a Handle<U> to a Handle<T>. T must provide a static member
/// method whose signature if 'T * T::handle_cast(U * u)'.
template<class T, class U>
Handle<T>
handle_cast(const Handle<U> & r)
{
    return handle_cast<T>(r.lock());
}

/// Converts a SharedPtr<U> to a Handle<T>. T must provide a static member
/// method whose signature if 'T * T::handle_cast(U * u)'.
template<class T, class U>
Handle<T>
handle_cast(const SharedPtr<U> & spu)
{
    if (spu.use_count() == 0)
    {
        return Handle<T>();
    }
    else
    {
        U * pu = spu.get();
        T * pt = T::handle_cast(pu);
        if (pt)
        {
            SharedPtr<T> spt(spu, pt);
            return Handle<T>(spt);
        }
        else
        {
            return Handle<T>();
        }
    }
}

/// \class Handle OpenVac/Core/Memory.h
/// \brief Extension of WeakPtr providing user-defined implicit upcasting and
///        downcasting.
///
/// <H1>Example</H1>
///
/// \code
/// class Base
/// {
/// public:
///     template <class Derived>
///     static Base * handle_cast(Derived * d) { return static_cast<Base*>(d); }
///
///     virtual void foo() { std::cout << "foo() from Base.\n"; }
/// };
///
/// class Derived1: public Base
/// {
/// public:
///     static Derived1 * handle_cast(Base * b) { return dynamic_cast<Derived1*>(b); }
///
///     void foo()  { std::cout << "foo() from Derived1.\n"; }
///     void bar1() { std::cout << "bar1().\n"; }
/// };
///
/// class Derived2: public Base
/// {
/// public:
///     static Derived2 * handle_cast(Base * b) { return dynamic_cast<Derived2*>(b); }
///
///     void foo()  { std::cout << "foo() from Derived2.\n"; }
///     void bar1() { std::cout << "bar2().\n"; }
/// };
///
/// class Unrelated {};
///
/// int main()
/// {
///     // Creating handles
///     auto sp1 = std::make_shared<Derived1>();
///     auto sp2 = std::make_shared<Derived1>();
///     Handle<Derived1> h1 = sp1;
///     Handle<Derived2> h2 = sp2;
///
///     // Downcasting
///     Handle<Base> h1_as_Base = h1;
///     Handle<Base> h2_as_Base = h2;
///     h1_as_Base->foo(); // Prints "foo() from Derived1"
///     h2_as_Base->foo(); // Prints "foo() from Derived2"
///
///     // Upcasting
///     Handle<Base> h = h1;
///     Handle<Derived1> maybe_h1 = h;
///     Handle<Derived2> maybe_h2 = h;
///     maybe_h1->foo1();    // Prints "Calling foo1."
///     // maybe_h2->foo2(); // Aborts at runtime
///     if (maybe_h1) maybe_h1->foo1(); // Prints "Calling foo1."
///     if (maybe_h2) maybe_h2->foo2(); // Prints nothing
///
///     // Casting to unrelated class
///     auto spu = std::make_shared<Unrelated>();
///     Handle<Unrelated> unrelated = spu;
///     // Handle<Derived2> h2_from_h1 = h1;               // Compile error
///     // Handle<Derived2> h2_from_unrelated = unrelated; // Compile error
/// }
/// \endcode

template <class T>
class Handle
{
public:
    using element_type = T;

    // Constructors
    Handle() : ptr_() {}

    Handle(const Handle & r) : ptr_(r.ptr_) {}

    template <class Y>
    Handle(const Handle<Y> & r) : Handle(handle_cast<T>(r)) {}

    Handle(const SharedPtr<T> & r) : ptr_(r) {}

    template <class Y>
    Handle(const SharedPtr<Y> & r) : Handle(handle_cast<T>(r)) {}

    // Destructor
    ~Handle() {}

    // Assignment operators
    Handle & operator=(const Handle & r) { ptr_ = r.ptr_; return *this; }

    template <class Y>
    Handle & operator=(const Handle<Y> & r)
    {
        Handle h(r);
        ptr_ = h.ptr_;
        return *this;
    }

    template <class Y>
    Handle & operator=(const SharedPtr<Y> & r)
    {
        Handle h(r);
        ptr_ = h.ptr_;
        return *this;
    }

    // Reset
    void reset() { ptr_.reset(); }

    // Swap
    void swap( Handle & r ) { ptr_.swap(r.ptr_); }

    // Use count
    long use_count() const { return ptr_.use_count(); }

    // Expired
    bool expired() const { return ptr_.expired(); }

    // Lock
    SharedPtr<T> lock() const { return ptr_.lock(); }

    // Owner before
    template <class Y>
    bool owner_before(const Handle<Y> & other) const { return ptr_.owner_before(other); }

    template <class Y>
    bool owner_before(const SharedPtr<Y> & other) const { return ptr_.owner_before(other); }

    // Conversion to bool
    operator bool() const { return (bool)ptr_; }

    // Dereferencing
    T * get() const { return ptr_.get(); }
    T & operator*() const { T * p = get(); assert(p); return *p; }
    T * operator->() const { T * p = get(); assert(p); return p; }

private:
    WeakPtr<T> ptr_;
};

template <class T>
void swap(Handle<T> & lhs, Handle<T> & rhs )
{
    return lhs.swap(rhs);
}

} // end namespace OpenVac

#endif // OPENVAC_MEMORY_H
