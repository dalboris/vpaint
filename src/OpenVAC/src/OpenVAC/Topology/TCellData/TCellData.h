// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_TCELLDATA_H
#define OPENVAC_TCELLDATA_H

#include <OpenVAC/Topology/CellType.h>

namespace OpenVAC
{

template <class T> class TKeyVertexData;
template <class T> class TKeyEdgeData;
template <class T> class TKeyFaceData;
template <class T> class TInbetweenVertexData;
template <class T> class TInbetweenEdgeData;
template <class T> class TInbetweenFaceData;

/// \class TCellData Topology/TCellData/TCellData.h
/// \brief TCellData is a class template to store low-level cell topological data.
///
/// The TCellData<T> classes are not meant to be used by client code. Instead,
/// you should use the VAC, Cell, and Operator classes, which are a safe and
/// user-friendly layer to access and modify cell data, through an ID-based
/// cell management sytem. Though, you may use the TCellData<T> classes if you
/// want to implement your own cell management sytem and client interface.
///
/// The 'TCellData<T> classes' refers to the following classes:
///     - TCellData<T>
///     - TKeyVertexData<T>
///     - TKeyEdgeData<T>
///     - TKeyFaceData<T>
///     - TInbetweenVertexData<T>
///     - TInbetweenEdgeData<T>
///     - TInbetweenFaceData<T>
///
/// TCellData<T> is an abstract base class which is inherited by all other
/// TCellData<T> classes.
///
/// The TCellData<T> classes store raw cell topological data, in a struct-like
/// fashion. They do not enforce topological consistency, and they are not
/// aware of any cell management system (e.g., they do not have an ID).
///
/// Note that Frame is considered "cell topological data" in OpenVAC. The
/// reason is that in the Vector Animation Complex, the time axis plays a very
/// special role, and despite being geometric in nature, cannot be decorrelated
/// from topology. For instance, the Frame attribute of two key vertices
/// connected by a key edge *must* be equal, therefore the validity of the
/// topological operator OpMakeKeyEdge depends on these Frame attributes, and
/// therefore they cannot be separated from topology.
///
/// The template parameter T allows you to customize what type is used to store
/// "references" to other cells (here, "references" is not used in the C++
/// sense). For instance, the topology of a key edge is defined by two key
/// vertices, and therefore one needs to be able to "refer" to them. One way
/// can be to use a raw C++ pointer, a C++ reference, a smart pointer, an
/// integer (as part od an ID management system), or maybe another customized
/// way.
///
/// See CellData and OpCellData as two examples of class instantiations of
/// TCellData. The template parameter of CellData is CellDataTrait, defining
/// CellRef as Cell*, and the template parameter of OpCellData is
/// OpCellDataTrait, defining CellRef as CellId (itself defined as unsigned
/// int).

template <class T>
class TCellData
{
public:
    // Virtual destructor
    virtual ~TCellData() {}

    // Type
    virtual CellType type() const=0;

    // Type casting to this class
    TCellData<T> * toCellData() { return this; }
    const TCellData<T> * toCellData() const { return this; }
    static TCellData<T> * cast(TCellData<T> * c) { return c ? c->toCellData() : nullptr; }
    static const TCellData<T> * cast(const TCellData<T> * c) { return c ? c->toCellData() : nullptr; }

    // Type casting to derived classes
    virtual TKeyVertexData<T> * toKeyVertexData()             { return nullptr; }
    virtual TKeyEdgeData<T> * toKeyEdgeData()                 { return nullptr; }
    virtual TKeyFaceData<T> * toKeyFaceData()                 { return nullptr; }
    virtual TInbetweenVertexData<T> * toInbetweenVertexData() { return nullptr; }
    virtual TInbetweenEdgeData<T> * toInbetweenEdgeData()     { return nullptr; }
    virtual TInbetweenFaceData<T> * toInbetweenFaceData()     { return nullptr; }
    virtual const TKeyVertexData<T> * toKeyVertexData() const             { return nullptr; }
    virtual const TKeyEdgeData<T> * toKeyEdgeData() const                 { return nullptr; }
    virtual const TKeyFaceData<T> * toKeyFaceData() const                 { return nullptr; }
    virtual const TInbetweenVertexData<T> * toInbetweenVertexData() const { return nullptr; }
    virtual const TInbetweenEdgeData<T> * toInbetweenEdgeData() const     { return nullptr; }
    virtual const TInbetweenFaceData<T> * toInbetweenFaceData() const     { return nullptr; }

    // Templated type casting. Equivalent to dynamic_cast but much faster.
    template <class U> U * to() { return U::cast(this); }
    template <class U> const U * to() const { return U::cast(this); }
};

}

#endif
