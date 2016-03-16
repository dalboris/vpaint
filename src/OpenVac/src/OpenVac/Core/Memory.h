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

/// \typedef OpenVac::UniquePtr OpenVac/Core/Memory.h
/// \brief Alias for std::unique_ptr
///
template <class T>
using UniquePtr = std::unique_ptr<T>;


/******************************** SharedPtr **********************************/

/// \typedef OpenVac::SharedPtr OpenVac/Core/Memory.h
/// \brief Alias for std::shared_ptr
///
template <class T>
using SharedPtr = std::shared_ptr<T>;


/******************************** WeakPtr **********************************/

/// \class WeakPtr OpenVac/Core/Memory.h
/// \brief Extension of std::weak_ptr
///
/// Extends std::weak_ptr by explicitely storing a raw pointer to the
/// managed object, allowing the WeakPtr to be used as a conventional
/// non-owning raw pointer at no performance cost.
///
/// In other words, if you know that by design, the stored pointer is
/// guaranteed to be non-null and alive, you can use "->" as with regular raw
/// pointers. If your assumption is wrong, this can cause undefined behaviour:
/// no check is done, the stored pointer is directly dereferenced.
///
/// \code
/// SharedPtr<Foo> sp = std::make_shared<Foo>();
/// WeakPtr<Foo> wp = sp;
/// wp->doSomething();
/// \endcode
///
/// If you know that by design, the stored pointer is guaranteed to be either
/// null, or non-null and alive, then you can test for null and use "->", as
/// with regular raw pointers. If your assumption is wrong, this can cause
/// undefined behaviour: the pointer might be not null, but the pointed object
/// might still have been deleted by its owner, possibly in a concurrent
/// thread.
///
/// \code
/// WeakPtr<Foo> foo = findFoo();
/// if (foo)
/// {
///     foo->doSomething();
/// }
/// \endcode
///
/// If you know that the object might have been deleted by its owner, then you can
/// take advantage of the features of std::weak_ptr to take temporary
/// ownership. This is always safe, but in most cases you shouldn't need it, and
/// design your software such that non-owning pointers are guaranteed upstream
/// to always be either null, or non-null and alive.
///
/// \code
/// WeakPtr<Foo> foo = findFoo();
/// maybeDelete(foo);
/// if (auto & sfoo = foo.lock()) // sfoo is a SharedPtr<Foo>
/// {
///     sfoo->doSomething();
/// }
/// else
/// {
///     doSomethingElse();
/// }
/// \endcode
///
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
    explicit operator bool() const { return p_; }

    // Dereferencing
    T * get() const { return p_; }
    T & operator*() const { return *get(); }
    T * operator->() const { return get(); }

    // Comparison with other smart pointer types
    // Note: lock and test equality between shared pointer instead
    // in cases where might be deleted.
    template <class YPtr>
    bool operator==(const YPtr & other)
    {
        return get() == other.get();
    }

    // XXX do I want to provide as well implicit casting to raw pointer?

private:
    std::weak_ptr<T> wp_;
    T * p_;
};

template <class T>
void swap(WeakPtr<T> & lhs, WeakPtr<T> & rhs)
{
    return lhs.swap(rhs);
}


/*********************************** Handle **********************************/

// Forward declaration of Handle class template
template <class T> class Handle;

/// Converts a Handle<U> to a Handle<T>. T must provide a static member
/// method whose signature if 'T * T::handle_cast(U * u)'.
///
template<class T, class U>
Handle<T>
handle_cast(const Handle<U> & r)
{
    return handle_cast<T>(r.lock());
}

/// Converts a SharedPtr<U> to a Handle<T>. T must provide a static member
/// method whose signature if 'T * T::handle_cast(U * u)'.
///
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
///
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
    explicit operator bool() const { return (bool)ptr_; }

    // Dereferencing
    T * get() const { return ptr_.get(); }
    T & operator*() const { return *get(); }
    T * operator->() const { return get(); }

    // Comparison with other smart pointer types
    template <class YPtr>
    bool operator==(const YPtr & other)
    {
        return get() == other.get();
    }

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
