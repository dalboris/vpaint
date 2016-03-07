// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_CELLHANDLE_H
#define OPENVAC_CELLHANDLE_H

#include <OpenVac/Core/Memory.h>
#include <OpenVac/Core/ForeachCellType.h>

// XXX this header should be removed
#include <OpenVac/Topology/CellType.h>

#define OPENVAC_DEFINE_STATIC_TO_CELL_(CellType) \
    static CellType<Geometry> * to##CellType(Cell * c) { return c ? c->to##CellType##_() : nullptr; }

#define OPENVAC_DEFINE_MEMBER_TO_CELL_(CellType) \
    virtual CellType<Geometry> * to##CellType##_() { return nullptr; }

#define OPENVAC_DEFINE_CELL_CAST_BASE \
    protected: OPENVAC_FOREACH_CELL_TYPE(OPENVAC_DEFINE_STATIC_TO_CELL_) \
    private:   OPENVAC_FOREACH_DERIVED_CELL_TYPE(OPENVAC_DEFINE_MEMBER_TO_CELL_)

#define OPENVAC_DEFINE_CELL_CAST(CellType) \
    template<class T, class U> friend TCellHandle<T> cell_handle_cast(const TCellHandle<U> & r); \
    template<class T, class U> friend TCellHandle<T> cell_handle_cast(const SharedPtr<U> & r); \
    CellType * to##CellType##_() { return this; } \
    static CellType * cast_(Cell<Geometry> * c) { return Cell<Geometry>::to##CellType(c); }

#define OPENVAC_FORWARD_DECLARE_CELL_(CellType) \
    template <class Geometry> \
    class CellType;

#define OPENVAC_DECLARE_CELL_SHARED_PTR_(CellType) \
    template <class Geometry> \
    using CellType##SharedPtr = SharedPtr<CellType<Geometry>>;

#define OPENVAC_DECLARE_CELL_HANDLE_(CellType) \
    template <class Geometry> \
    using CellType##Handle = TCellHandle<CellType<Geometry>>;

namespace OpenVac
{

/////////////////////////    Define cell_handle_cast    ///////////////////////

// Forward declaration of TCellHandle class template
template <class T> class TCellHandle;

// Casting from TCellHandle<U> to TCellHandle<T>
template<class T, class U>
TCellHandle<T>
cell_handle_cast(const TCellHandle<U> & r)
{
    return cell_handle_cast<T>(r.lock());
}

// Casting from SharedPtr<U> to TCellHandle<T>
template<class T, class U>
TCellHandle<T>
cell_handle_cast(const SharedPtr<U> & spu)
{
    if (spu.use_count() == 0)
    {
        return TCellHandle<T>();
    }
    else
    {
        U * pu = spu.get();
        //T * pt = dynamic_cast<T*>(pu);
        // XXX
        T * pt = T::cast_(pu);
        if (pt)
        {
            SharedPtr<T> spt(spu, pt);
            return TCellHandle<T>(spt);
        }
        else
        {
            return TCellHandle<T>();
        }
    }
}

///////////////////    Define TCellHandle class template    ///////////////////

template <class T>
class TCellHandle
{
public:
    typedef T element_type;

    // Constructors
    TCellHandle() : ptr_() {}

    TCellHandle(const TCellHandle & r) : ptr_(r.ptr_) {}

    template <class Y>
    TCellHandle(const TCellHandle<Y> & r) : TCellHandle(cell_handle_cast<T>(r))
    {
        if (r)
        {
            CellType t = r->type();
            CellType t2 = t;
        }
    }

    TCellHandle(const SharedPtr<T> & r) : ptr_(r) {}

    template <class Y>
    TCellHandle(const SharedPtr<Y> & r) : TCellHandle(cell_handle_cast<T>(r)) {}

    // Destructor
    ~TCellHandle() {}

    // Assignment operators
    TCellHandle & operator=(const TCellHandle & r) { ptr_ = r.ptr_; return *this; }

    template <class Y>
    TCellHandle & operator=(const TCellHandle<Y> & r)
    {
        TCellHandle h(r);
        ptr_ = h.ptr_;
        return *this;
    }

    template <class Y>
    TCellHandle & operator=(const SharedPtr<Y> & r)
    {
        TCellHandle h(r);
        ptr_ = h.ptr_;
        return *this;
    }

    // Reset
    void reset() { ptr_.reset(); }

    // Swap
    void swap( TCellHandle & r ) { ptr_.swap(r.ptr_); }

    // Use count
    long use_count() const { return ptr_.use_count(); }

    // Expired
    bool expired() const { return ptr_.expired(); }

    // Lock
    SharedPtr<T> lock() const { return ptr_.lock(); }

    // Owner before
    template <class Y>
    bool owner_before(const TCellHandle<Y> & other) const { return ptr_.owner_before(other); }

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
void swap(TCellHandle<T> & lhs, TCellHandle<T> & rhs )
{
    return lhs.swap(rhs);
}

OPENVAC_FOREACH_CELL_TYPE(OPENVAC_FORWARD_DECLARE_CELL_)
OPENVAC_FOREACH_CELL_TYPE(OPENVAC_DECLARE_CELL_SHARED_PTR_)
OPENVAC_FOREACH_CELL_TYPE(OPENVAC_DECLARE_CELL_HANDLE_)

}

#endif // OPENVAC_CELLHANDLE_H
