// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_CELLDATAMUTATOR_H
#define OPENVAC_CELLDATAMUTATOR_H

#include <OpenVac/Core/ForeachCellType.h>
#include <OpenVac/Core/CellType.h>

namespace OpenVac
{

/********************** Forward declare CellData classes **********************/

#define OPENVAC_CELLDATAMUTATOR_FORWARD_DECLARE_CELL_DATA_(CellType) \
    template <class T> \
    class CellType##Data;

OPENVAC_FOREACH_CELL_TYPE(OPENVAC_CELLDATAMUTATOR_FORWARD_DECLARE_CELL_DATA_)


/**************************** CellDataMutator ********************************/

/// \class CellDataMutator OpenVac/Data/Util/CellDataMutator.h
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
template <class T>
class CellDataMutator
{
public:
    /// Calls the visit() virtual function corresponding to the dynamic type
    /// of \p data.
    void visit(CellData<T> & data)
    {
        data.accept(*this);
    }

    #define OPENVAC_CELLDATA_DECLARE_VISIT_(CellType_) \
        /** This virtual function is called by the non-virtual visit() */ \
        /** function whenever `data.type() == CellType::##CellType_`.  */ \
        /** Reimplement it in subclasses. The default implementation   */ \
        /** does nothing.                                              */ \
        virtual void visit(CellType_##Data<T> & data) {}

    OPENVAC_FOREACH_FINAL_CELL_TYPE(OPENVAC_CELLDATA_DECLARE_VISIT_)
};

} // end namespace OpenVac

#endif // OPENVAC_CELLDATAMUTATOR_H
