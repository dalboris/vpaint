// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_OPCELLDATACONVERTER_H
#define OPENVAC_OPCELLDATACONVERTER_H

#include <OpenVac/Core/CellId.h>
#include <OpenVac/Core/ForeachCellType.h>
#include <OpenVac/Data/UsingData.h>
#include <OpenVac/Data/CellData.h>
#include <OpenVac/Data/KeyVertexData.h>
#include <OpenVac/Data/KeyEdgeData.h>
#include <OpenVac/Topology/Cell.h>
#include <OpenVac/Operators/Operator.h>
#include <OpenVac/Vac.h>

namespace OpenVac
{


/***************************** CellRefTranslator *****************************/

template <class T, class U>
class CellRefTranslator
{
public:
    #define OPENVAC_CELLDATACOPIER_DECLARE_TRANSLATE_(CellType) \
        virtual void translate( \
            const typename T::CellType##Ref & from, \
                  typename U::CellType##Ref & to  )=0;

    OPENVAC_FOREACH_CELL_TYPE(OPENVAC_CELLDATACOPIER_DECLARE_TRANSLATE_)
};


/*************************** IdToHandleTranslator ****************************/

class UsingCellIdsAsCellRefs;
template <class Geometry> class UsingCellHandlesAsCellRefs;

template <class Geometry>
class IdToHandleTranslator: public CellRefTranslator<
                                       UsingCellIdsAsCellRefs,
                                       UsingCellHandlesAsCellRefs<Geometry> >
{
private:
    using Vac = OpenVac::Vac<Geometry>;
    const Vac * vac_;

public:
    IdToHandleTranslator(const Vac * vac) : vac_(vac) {}

    #define OPENVAC_CELLDATACOPIER_DEFINE_ID_TO_HANDLE_(CellType) \
        using CellType##Handle = Handle< CellType<Geometry> >; \
        \
        void translate(const CellType##Id     & id, \
                             CellType##Handle & handle) \
        { \
            handle = vac_->cell(id); \
        }

    OPENVAC_FOREACH_CELL_TYPE(OPENVAC_CELLDATACOPIER_DEFINE_ID_TO_HANDLE_)
};


/*************************** HandleToIdTranslator ****************************/

template <class Geometry>
class HandleToIdTranslator: public CellRefTranslator<
                                       UsingCellHandlesAsCellRefs<Geometry>,
                                       UsingCellIdsAsCellRefs >
{
public:
    HandleToIdTranslator() {}

    #define OPENVAC_CELLDATACOPIER_DEFINE_HANDLE_TO_ID_(CellType) \
        using CellType##Handle = Handle< CellType<Geometry> >; \
        \
        void translate(const CellType##Handle & handle, \
                             CellType##Id     & id    ) \
        { \
            id = handle->id(); \
        }

    OPENVAC_FOREACH_CELL_TYPE(OPENVAC_CELLDATACOPIER_DEFINE_HANDLE_TO_ID_)
};


/*************************** CellDataCopierVisitor ***************************/

template <class T, class U, class Geometry>
class CellDataCopierVisitor: public CellDataVisitor<T, Geometry>
{
private:
    // Type aliases
    OPENVAC_USING_DATA(T, T, Geometry) // using TCellData = CellData<T, Geometry>
    OPENVAC_USING_DATA(U, U, Geometry) // using UCellData = CellData<U, Geometry>
    using CellRefTranslator = OpenVac::CellRefTranslator<T, U>;

    // Member variables
    CellRefTranslator * cellRefTranslator_;
    UCellData * to_;

    // Upcasts 'to_'
    template <class UCellTypeData>
    UCellTypeData & getTo()
    {
        return static_cast<UCellTypeData&>(*to_);
    }

    // Convenient forwarding functions
    template <class TCellTypeRef, class UCellTypeRef>
    void translateRef(const TCellTypeRef & from, UCellTypeRef & to)
    {
        cellRefTranslator_->translate(from, to);
    }

public:
    CellDataCopierVisitor(CellRefTranslator * cellRefTranslator, UCellData * to) :
        cellRefTranslator_(cellRefTranslator),
        to_(to)
    {
    }

    void visit(const TKeyVertexData & from)
    {
        UKeyVertexData & to = getTo<UKeyVertexData>();

        to.frame    = from.frame;
        to.geometry = from.geometry;
    }

    void visit(const TKeyEdgeData & from)
    {
        UKeyEdgeData & to = getTo<UKeyEdgeData>();

        translateRef(from.startVertex, to.startVertex);
        translateRef(from.endVertex,   to.endVertex);

        to.frame    = from.frame;
        to.geometry = from.geometry;
    }
};


/****************************** CellDataCopier *******************************/

template <class T, class U, class Geometry>
class CellDataCopier
{
private:
    // Type aliases
    using TCellData = CellData<T, Geometry>;
    using UCellData = CellData<U, Geometry>;
    using Visitor = CellDataCopierVisitor<T, U, Geometry>;
    using CellRefTranslator = OpenVac::CellRefTranslator<T, U>;

    // Member variable
    CellRefTranslator * cellRefTranslator_;

public:
    CellDataCopier(CellRefTranslator * cellRefTranslator) :
        cellRefTranslator_(cellRefTranslator)
    {
    }

    void copy(const TCellData & from, UCellData & to)
    {
        assert(from.type() == to.type());
        Visitor visitor(cellRefTranslator_, &to);
        from.accept(visitor);
    }
};


/**************************** OpToCellDataCopier *****************************/

template <class Geometry>
class OpToCellDataCopier
{
private:
    // Type aliases
    using T = UsingCellIdsAsCellRefs;
    using U = UsingCellHandlesAsCellRefs<Geometry>;
    using TCellData = CellData<T, Geometry>;
    using UCellData = CellData<U, Geometry>;
    using Vac = OpenVac::Vac<Geometry>;

    // Member variables
    IdToHandleTranslator<Geometry> refTranslator_;
    CellDataCopier<T, U, Geometry> copier_;

public:
    OpToCellDataCopier(const Vac * vac) :
        refTranslator_(vac),
        copier_(&refTranslator_)
    {
    }

    void copy(const TCellData & from, UCellData & to)
    {
        copier_.copy(from, to);
    }
};


/**************************** CellToOpDataCopier *****************************/

template <class Geometry>
class CellToOpDataCopier
{
private:
    // Type aliases
    using T = UsingCellHandlesAsCellRefs<Geometry>;
    using U = UsingCellIdsAsCellRefs;
    using TCellData = CellData<T, Geometry>;
    using UCellData = CellData<U, Geometry>;
    using Vac = OpenVac::Vac<Geometry>;

    // Member variables
    HandleToIdTranslator<Geometry> refTranslator_;
    CellDataCopier<T, U, Geometry> copier_;

public:
    CellToOpDataCopier() :
        refTranslator_(),
        copier_(&refTranslator_)
    {
    }

    void copy(const TCellData & from, UCellData & to)
    {
        copier_.copy(from, to);
    }
};


} // end namespace OpenVac

#endif
