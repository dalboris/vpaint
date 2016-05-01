// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_CELLDATA_H
#define OPENVAC_CELLDATA_H

#include <OpenVac/Core/CellType.h>
#include <OpenVac/Data/Util/CellDataVisitor.h>
#include <OpenVac/Data/Util/CellDataMutator.h>

namespace OpenVac
{

/// \class CellData OpenVac/Data/CellData.h
/// \brief A class to store cell raw data.
///
/// The "Data classes" (all the classes defined in OpenVac/Data/*) are not meant
/// to be created or modified directly by client code. Instead, you should use
/// the Vac, Cell, and Operator classes, which are a safe and user-friendly
/// layer to create, modify, and access cell data, through an ID-based cell
/// management sytem. Though, you may use the Data classes if you want to
/// implement your own cell management sytem and client interface (i.e., not
/// use OpenVac/Vac.h and OpenVac/Topology/*).
///
/// CellData is an abstract base class which is inherited by the following
/// classes:
///     * KeyVertexData
///     * KeyEdgeData
///     * KeyFaceData
///     * InbetweenVertexData
///     * InbetweenEdgeData
///     * InbetweenFaceData
///
/// These classes are responsible to store raw cell data (both topological data
/// and geometric data), in a struct-like fashion. They do not enforce
/// topological consistency, and they are not aware of any cell management
/// system (e.g., they do not have an ID).
///
/// All topological data is built-in, but you are responsible to define your
/// own geometric data via the template parameter Geometry. This geometric data
/// is stored as an attribute (public member variable) named \p geometry,
/// declared in each class derived from CellData. For instance, KeyVertexData
/// declares the \p geometry attribute of type Geometry::KeyVertexGeometry.
///
/// Also built-in is the \p frame attribute of key cells, so you don't have to
/// define it yourself, despite being geometric data. The reason it's built-in
/// is that unfortunately, geometry and topology are coupled via this frame
/// attribute. This is not a design decision but a fundamental property of the
/// Vector Animation Complex which we have to live with. For instance, the two
/// end vertices of a key edge must have the same frame value, and therefore
/// OpMakeKeyEdge::isValid() depends on KeyVertexData::frame. This means that
/// OpenVac won't let you freely change this attribute, as it would corrupt the
/// data structure. You need to use the topological operator OpSetFrame for
/// that purpose.
///
/// Though, despite being built-in, its type is Geometry::Frame, so at least
/// this is in your control. Two typical types you may want to use is \p int
/// and the provided DFrame.
///
/// Finally, the template parameter T is an internal implementation detail that
/// allows to customize what type is used to "refer" to other cells. If you are
/// a typical user (i.e., using the Vac and Operator classes to create and
/// modify a Vector Animation Complex), then you are only exposed to CellData
/// via Cell::data() which uses T = UsingCellHandlesAsCellRefs. This means
/// that T::CellRef is an alias for CellHandle, T::KeyVertexRef is an alias
/// fo KeyVertexHandle, etc. Example:
///
/// \code
/// KeyEdgeHandle keyEdge = vac->cell(id);
/// KeyVertexHandle keyVertex = keyEdge->data().startVertex; // same as keyEdge->startVertex()
/// \endcode
///
/// However, if you are writing your own subclass of Operator, then you are
/// exposed to cell data differently. In your reimplementation of the virtual
/// method compute_(), then you also manipulate "OpCellData", which is a
/// CellData that uses T = UsingCellIdsAsCellRefs. This means that T::CellRef
/// is an alias for CellId, T::KeyVertexRef is an alias fo KeyVertexId, etc.,
/// which are all aliases for unsigned int. For more info, see the
/// documentation for Operator.
///
/// If you are an atypical user of the library which only use <OpenVac/Data/*>
/// and ignores all the convenient Vac and Operator classes, then feel free to
/// use T as you wish. For instance, all reference types may be CellData*.
///
template <class T>
class CellData
{
public:
    /// Virtual destructor that makes it safe to destruct an object of a
    /// subclass via a pointer of the base class.
    virtual ~CellData() {}

    /// Returns CellType::Cell. This static function is reimplemented in
    /// derived classes to return their associated CellType.
    static CellType static_type() { return CellType::Cell; }

    /// Returns the dynamic CellType associated with this cell data.
    virtual CellType type() const=0;

    /// Implements double-dispatch via the Visitor pattern. See CellDataVisitor
    /// for more information.
    virtual void accept(CellDataVisitor<T> & visitor) const=0;

    /// Implements double-dispatch via the Visitor pattern. See CellDataMutator
    /// for more information.
    virtual void accept(CellDataMutator<T> & mutator)      =0;
};

} // end namespace OpenVac

#endif
