// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_CELLHANDLE_H
#define OPENVAC_CELLHANDLE_H

#include <OpenVAC/Core/Memory.h>

namespace OpenVAC
{

////////////////////    Definition of cell_handle_cast    ///////////////////

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

///////////////    Definition of TCellHandle class template    ////////////////

template <class T>
class TCellHandle
{
public:
    // Constructors
    TCellHandle() : ptr_() {}

    TCellHandle(const TCellHandle & r) : ptr_(r.ptr_) {}

    template <class Y>
    TCellHandle(const TCellHandle<Y> & r) : TCellHandle(cell_handle_cast<T>(r)) {}

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
    T & operator*() const { return *get(); }
    T * operator->() const { return get(); }

private:
    WeakPtr<T> ptr_;
};

template <class T>
void swap(TCellHandle<T> & lhs, TCellHandle<T> & rhs )
{
    return lhs.swap(rhs);
}

///////////////////    Declaration of CellHandle classes    ///////////////////

#define OPENVAC_DECLARE_CELL_HANDLE(CellType) \
    class CellType; \
    typedef OpenVAC::TCellHandle<CellType> CellType##Handle;

OPENVAC_DECLARE_CELL_HANDLE(Cell)
OPENVAC_DECLARE_CELL_HANDLE(KeyCell)
OPENVAC_DECLARE_CELL_HANDLE(InbetweenCell)
OPENVAC_DECLARE_CELL_HANDLE(VertexCell)
OPENVAC_DECLARE_CELL_HANDLE(EdgeCell)
OPENVAC_DECLARE_CELL_HANDLE(FaceCell)
OPENVAC_DECLARE_CELL_HANDLE(KeyVertex)
OPENVAC_DECLARE_CELL_HANDLE(KeyEdge)
OPENVAC_DECLARE_CELL_HANDLE(KeyFace)
OPENVAC_DECLARE_CELL_HANDLE(InbetweenVertex)
OPENVAC_DECLARE_CELL_HANDLE(InbetweenEdge)
OPENVAC_DECLARE_CELL_HANDLE(InbetweenFace)

//////////////////    Declaration of CellSharedPtr classes    /////////////////

OPENVAC_DECLARE_SHARED_PTR(Cell)
OPENVAC_DECLARE_SHARED_PTR(KeyCell)
OPENVAC_DECLARE_SHARED_PTR(InbetweenCell)
OPENVAC_DECLARE_SHARED_PTR(VertexCell)
OPENVAC_DECLARE_SHARED_PTR(EdgeCell)
OPENVAC_DECLARE_SHARED_PTR(FaceCell)
OPENVAC_DECLARE_SHARED_PTR(KeyVertex)
OPENVAC_DECLARE_SHARED_PTR(KeyEdge)
OPENVAC_DECLARE_SHARED_PTR(KeyFace)
OPENVAC_DECLARE_SHARED_PTR(InbetweenVertex)
OPENVAC_DECLARE_SHARED_PTR(InbetweenEdge)
OPENVAC_DECLARE_SHARED_PTR(InbetweenFace)

/////////////////////    Macros to define cell casting    /////////////////////

#define OPENVAC_DEFINE_CELL_CAST_BASE \
    protected: \
    static Cell * toCell(Cell * c)                       { return c ? c->toCell_()            : nullptr; } \
    static KeyCell * toKeyCell(Cell * c)                 { return c ? c->toKeyCell_()         : nullptr; } \
    static InbetweenCell * toInbetweenCell(Cell * c)     { return c ? c->toInbetweenCell_()   : nullptr; } \
    static VertexCell * toVertexCell(Cell * c)           { return c ? c->toVertexCell_()      : nullptr; } \
    static EdgeCell * toEdgeCell(Cell * c)               { return c ? c->toEdgeCell_()        : nullptr; } \
    static FaceCell * toFaceCell(Cell * c)               { return c ? c->toFaceCell_()        : nullptr; } \
    static KeyVertex * toKeyVertex(Cell * c)             { return c ? c->toKeyVertex_()       : nullptr; } \
    static KeyEdge * toKeyEdge(Cell * c)                 { return c ? c->toKeyEdge_()         : nullptr; } \
    static KeyFace * toKeyFace(Cell * c)                 { return c ? c->toKeyFace_()         : nullptr; } \
    static InbetweenVertex * toInbetweenVertex(Cell * c) { return c ? c->toInbetweenVertex_() : nullptr; } \
    static InbetweenEdge * toInbetweenEdge(Cell * c)     { return c ? c->toInbetweenEdge_()   : nullptr; } \
    static InbetweenFace * toInbetweenFace(Cell * c)     { return c ? c->toInbetweenFace_()   : nullptr; } \
    private: \
    virtual KeyCell * toKeyCell_()                 { return nullptr; } \
    virtual InbetweenCell * toInbetweenCell_()     { return nullptr; } \
    virtual VertexCell * toVertexCell_()           { return nullptr; } \
    virtual EdgeCell * toEdgeCell_()               { return nullptr; } \
    virtual FaceCell * toFaceCell_()               { return nullptr; } \
    virtual KeyVertex * toKeyVertex_()             { return nullptr; } \
    virtual KeyEdge * toKeyEdge_()                 { return nullptr; } \
    virtual KeyFace * toKeyFace_()                 { return nullptr; } \
    virtual InbetweenVertex * toInbetweenVertex_() { return nullptr; } \
    virtual InbetweenEdge * toInbetweenEdge_()     { return nullptr; } \
    virtual InbetweenFace * toInbetweenFace_()     { return nullptr; }

#define OPENVAC_DEFINE_CELL_CAST(CellType) \
    template<class T, class U> \
    friend TCellHandle<T> cell_handle_cast(const TCellHandle<U> & r); \
    template<class T, class U> \
    friend TCellHandle<T> cell_handle_cast(const SharedPtr<U> & r); \
    CellType * to##CellType##_() { return this; } \
    static CellType * cast_(Cell * c) { return to##CellType(c); }

}

#endif // OPENVAC_CELLHANDLE_H
