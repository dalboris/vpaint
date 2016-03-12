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
#include <OpenVac/Core/ForeachCellType.h>

namespace OpenVac
{

/********************** Forward declare CellData classes **********************/

#define OPENVAC_CELLDATA_FORWARD_DECLARE_CELL_DATA_(CellType) \
    template <class T, class Geometry> \
    class CellType##Data;

OPENVAC_FOREACH_CELL_TYPE(OPENVAC_CELLDATA_FORWARD_DECLARE_CELL_DATA_)


/**************************** CellDataVisitor ********************************/

/// \class CellDataVisitor OpenVac/Data/CellData.h
/// \brief A class that implements dynamic dispatch for CellData using the
/// Visitor pattern.
///
/// The CellDataVisitor class should be used whenever you need to do something
/// with a CellData& that depends on its actual derived type, but you don't
/// know this type at compile time.
///
/// In other words, whenever you feel the need to write code like:
///
/// \code
/// if (cellData.type() == CellType::KeyVertex)
/// {
///    KeyVertexData & keyVertexData = static_cast<KeyVertexData &> (cellData);
///    // ...
/// }
/// else if
/// // ...
/// \endcode
///
/// Then you should instead subclass CellDataVisitor and reimplement its
/// visit() virtual functions.
///
/// Note that CellDataVisitor does not allow you to modify the data. If you
/// need to modify the data, use CellDataMutator instead.
///
/// <H2>Example:</H2>
///
/// \code
/// class CellDataPrinter: public MyVac::CellDataVisitor
/// {
/// public:
///     void print(const MyVac::CellData & data) const
///     {
///         visit(data);
///     }
///
///     void visit(const MyVac::KeyVertexData & data) const
///     {
///         std::cout << "KeyVertexData( "
///                   <<     "pos = ("
///                   <<         data.geometry.pos[0] << ", " << data.geometry.pos[1]
///                   <<     ")";
///                   << " )\n";
///     }
///
///     void visit(const MyVac::KeyEdgeData & data) const
///     {
///         std::cout << "KeyEdgeData( "
///                   <<     "startVertex = " << data.startVertex->id() << " ; "
///                   <<     "endVertex = "   << data.endVertex->id()
///                   << " )\n";
///     }
/// }
///
/// void print(const MyVac::CellData & data)
/// {
///     CellDataPrinter().print(data);
/// }
/// \endcode
///
template <class T, class Geometry>
class CellDataVisitor
{
public:
    /// Calls the visit() virtual function corresponding to the dynamic type
    /// of \p data.
    void visit(const CellData<T, Geometry> & data)
    {
        data.accept(*this);
    }

    #define OPENVAC_CELLDATA_DECLARE_CONST_VISIT_(CellType_) \
        /** This virtual function is called by the non-virtual visit() */ \
        /** function whenever data.type() == CellType::##CellType_.    */ \
        /** Reimplement it in subclasses. The default implementation   */ \
        /** does nothing.                                              */ \
        virtual void visit(const CellType_##Data<T, Geometry> & data) {}

    OPENVAC_FOREACH_FINAL_CELL_TYPE(OPENVAC_CELLDATA_DECLARE_CONST_VISIT_)
};

/// \class CellDataMutator OpenVac/Data/CellData.h
/// \brief Same as CellDataVisitor, but allows to modify the data.
///
/// CellDataMutator is identical to CellDataVisitor with the exception that its
/// visit() functions allows to modify the data (it is passed by reference
/// instead of const reference).
///
/// See CellDataVisitor for more information.
///
/// <H2>Example:</H2>
///
/// \code
/// class CellDataAffineTransformer: public MyVac::CellDataMutator
/// {
/// private:
///     const AffineTransform & xf_;
///
/// public:
///     CellDataAffineTransformer(const AffineTransform & xf) : xf_(xf) {}
///
///     void transform(MyVac::CellData & data) const
///     {
///         visit(data);
///     }
///
///     void visit(MyVac::KeyVertexData & data) const
///     {
///         data.geometry.pos = xf_ * data.geometry.pos;
///     }
///
///     void visit(MyVac::KeyEdgeData & data) const
///     {
///         int n = data.geometry.curve.numSamples();
///         for (int i=0; i<n; ++i)
///         {
///             data.geometry.curve[i] = xf_ * data.geometry.curve[i];
///         }
///     }
/// }
///
/// void transform(MyVac::CellData & data, const AffineTransform & xf)
/// {
///     CellDataAffineTransformer(xf).transform(data);
/// }
/// \endcode
///
template <class T, class Geometry>
class CellDataMutator
{
public:
    /// Calls the visit() virtual function corresponding to the dynamic type
    /// of \p data.
    void visit(CellData<T, Geometry> & data)
    {
        data.accept(*this);
    }

    #define OPENVAC_CELLDATA_DECLARE_VISIT_(CellType_) \
        /** This virtual function is called by the non-virtual visit() */ \
        /** function whenever `data.type() == CellType::##CellType_`.  */ \
        /** Reimplement it in subclasses. The default implementation   */ \
        /** does nothing.                                              */ \
        virtual void visit(CellType_##Data<T, Geometry> & data) {}

    OPENVAC_FOREACH_FINAL_CELL_TYPE(OPENVAC_CELLDATA_DECLARE_VISIT_)
};


/******************************* CellData ************************************/

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
template <class T, class Geometry>
class CellData
{
public:
    // Virtual destructor
    virtual ~CellData() {}

    // Type
    virtual CellType type() const=0;

    // Visitor pattern
    virtual void accept(CellDataVisitor<T, Geometry> & visitor) const=0;
    virtual void accept(CellDataMutator<T, Geometry> & mutator)      =0;
};


/***************************** new_cell_data *********************************/

/// Allocate a CellData of the given \p type via \p new. Returns null if the
/// cell type is not a final type.
///
template <class T, class Geometry>
CellData<T, Geometry> * new_cell_data(CellType type)
{
    #define OPENVAC_CELLDATA_RETURN_NEW_IF_TYPE_MATCHES_(CellType_) \
        if (type == CellType::CellType_) \
            return new CellType_##Data<T, Geometry>();

    OPENVAC_FOREACH_FINAL_CELL_TYPE(
                OPENVAC_CELLDATA_RETURN_NEW_IF_TYPE_MATCHES_)

    return nullptr;
}


} // end namespace OpenVac

#endif
