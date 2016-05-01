// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_CELLDATAVISITOR_H
#define OPENVAC_CELLDATAVISITOR_H

#include <OpenVac/Core/ForeachCellType.h>
#include <OpenVac/Core/CellType.h>

namespace OpenVac
{

/********************** Forward declare CellData classes **********************/

#define OPENVAC_CELLDATAVISITOR_FORWARD_DECLARE_CELL_DATA_(CellType) \
    template <class T> \
    class CellType##Data;

OPENVAC_FOREACH_CELL_TYPE(OPENVAC_CELLDATAVISITOR_FORWARD_DECLARE_CELL_DATA_)


/**************************** CellDataVisitor ********************************/

/// \class CellDataVisitor OpenVac/Data/Util/CellDataVisitor.h
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
template <class T>
class CellDataVisitor
{
public:
    /// Calls the visit() virtual function corresponding to the dynamic type
    /// of \p data.
    void visit(const CellData<T> & data)
    {
        data.accept(*this);
    }

    #define OPENVAC_CELLDATA_DECLARE_CONST_VISIT_(CellType_) \
        /** This virtual function is called by the non-virtual visit() */ \
        /** function whenever data.type() == CellType::##CellType_.    */ \
        /** Reimplement it in subclasses. The default implementation   */ \
        /** does nothing.                                              */ \
        virtual void visit(const CellType_##Data<T> & data) {}

    OPENVAC_FOREACH_FINAL_CELL_TYPE(OPENVAC_CELLDATA_DECLARE_CONST_VISIT_)
};

} // end namespace OpenVac

#endif // OPENVAC_CELLDATAVISITOR_H
