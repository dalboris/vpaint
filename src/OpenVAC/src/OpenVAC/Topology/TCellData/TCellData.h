// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_TCELLDATA_H
#define OPENVAC_TCELLDATA_H

#include <OpenVAC/Core/ForeachCellType.h>
#include <OpenVAC/Topology/CellType.h>

#define OPENVAC_FORWARD_DECLARE_TCELL_DATA_(CellType) \
    template <class T, class Geometry> class T##CellType##Data;

#define OPENVAC_DEFINE_CELLDATA_CAST_BASE_(CellType) \
    virtual T##CellType##Data<T, Geometry> * to##CellType##Data() { return nullptr; } \
    virtual const T##CellType##Data<T, Geometry> * to##CellType##Data() const { return nullptr; }

#define OPENVAC_DEFINE_CELLDATA_CAST_BASE \
    OPENVAC_FOREACH_FINAL_CELL_TYPE(OPENVAC_DEFINE_CELLDATA_CAST_BASE_)

#define OPENVAC_DEFINE_CELLDATA_CAST(CellType) \
    T##CellType##Data<T, Geometry> * to##CellType##Data() { return this; } \
    const T##CellType##Data<T, Geometry> * to##CellType##Data() const { return this; } \
    static T##CellType##Data<T, Geometry> * cast(TCellData<T, Geometry> * c) { return c ? c->to##CellType##Data() : nullptr; } \
    static const T##CellType##Data<T, Geometry> * cast(const T##CellType##Data<T, Geometry> * c) { return c ? c->to##CellType##Data() : nullptr; }

namespace OpenVAC
{

OPENVAC_FOREACH_FINAL_CELL_TYPE(OPENVAC_FORWARD_DECLARE_TCELL_DATA_)

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

template <class T, class Geometry>
class TCellData
{
public:
    // Virtual destructor
    virtual ~TCellData() {}

    // Type
    virtual CellType type() const=0;

    // Type casting
    OPENVAC_DEFINE_CELLDATA_CAST_BASE
    OPENVAC_DEFINE_CELLDATA_CAST(Cell)
};

}

#endif
