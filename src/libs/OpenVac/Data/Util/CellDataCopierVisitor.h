// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_CELLDATACOPIERVISITOR_H
#define OPENVAC_CELLDATACOPIERVISITOR_H

#include <OpenVac/Data/Util/CellDataVisitor.h>
#include <OpenVac/Data/Util/CellRefTranslator.h>
#include <OpenVac/Data/CellData.h>
#include <OpenVac/Data/KeyVertexData.h>
#include <OpenVac/Data/KeyEdgeData.h>

namespace OpenVac
{

/// \class CellDataCopierVisitor OpenVac/Data/Util/CellDataCopierVisitor.h
/// \brief Subclass of CellDataVisitor to copy data from a CellData<T> into a
/// CellData<U>.
///
/// This sublcass of CellDataVisitor is where the actual work of CellDataCopier
/// is implemented. For each cell data type, its corresponding visit() function
/// copies all the data from one cell data to another, translating the
/// references in the process.
///
/// The CellData to write to is given as argument of the constructor, and
/// stored as a member variable. The CellData to read from is given as argument
/// to the visit() function. Once dispactched in the appropriate visit()
/// overload, the CellData to write to is manually casted, via static_cast, to
/// the same type as the CellDatavia to read from.
///
/// CellDataCopierVisitor assumes that the CellData to read from and the
/// CellData to write to have the same type.
///
template <class T, class U>
class CellDataCopierVisitor: public CellDataVisitor<T>
{
private:
    // Member variables
    CellRefTranslator<T, U> * translator_;
    CellData<U> * to_;

    // Upcasts the CellData to write to.
    template <class UCellTypeData>
    UCellTypeData & getTo()
    {
        return static_cast<UCellTypeData&>(*to_);
    }

    // Translates the cell references using the given translator.
    template <class TCellTypeRef, class UCellTypeRef>
    void translateRef(const TCellTypeRef & from, UCellTypeRef & to)
    {
        translator_->translate(from, to);
    }

public:
    /// Constructs a CellDataCopierVisitor.
    CellDataCopierVisitor(CellRefTranslator<T, U> * translator, CellData<U> * to) :
        translator_(translator),
        to_(to)
    {
    }

    /// Copies a KeyVertexData<T> into a KeyVertexData<U>.
    void visit(const KeyVertexData<T> & from)
    {
        KeyVertexData<U> & to = getTo<KeyVertexData<U>>();

        to.frame    = from.frame;
        to.geometry = from.geometry;
    }

    /// Copies a KeyEdgeData<T> into a KeyEdgeData<U>.
    void visit(const KeyEdgeData<T> & from)
    {
        KeyEdgeData<U> & to = getTo<KeyEdgeData<U>>();

        translateRef(from.startVertex, to.startVertex);
        translateRef(from.endVertex,   to.endVertex);

        to.frame    = from.frame;
        to.geometry = from.geometry;
    }
};

} // end namespace OpenVac

#endif // OPENVAC_CELLDATACOPIERVISITOR_H
