// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_OPCELLDATACONVERTER_H
#define OPENVAC_OPCELLDATACONVERTER_H

#include <OpenVAC/Topology/CellData.h>
#include <OpenVAC/Operators/OpCellData.h>
#include <OpenVAC/Topology/Cell.h>
#include <OpenVAC/Topology/KeyCell.h>
#include <OpenVAC/Topology/VertexCell.h>
#include <OpenVAC/Topology/EdgeCell.h>
#include <OpenVAC/Topology/KeyVertex.h>
#include <OpenVAC/Topology/KeyEdge.h>
#include <OpenVAC/VAC.h>

#define OPENVAC_IF_TYPE_MATCHES_THEN_CALL_CONVERT_(CellType_) \
    if (type == CellType::CellType_) \
    { \
        convert(*fromData.to##CellType_##Data(), *toData.to##CellType_##Data()); \
        return; \
    }

#define OPENVAC_DECLARE_CONVERT_(CellType) \
    virtual void convert_(const typename T::CellType##Ref & fromRef, typename U::CellType##Ref & toRef)=0;

#define OPENVAC_DEFINE_CELLID_TO_CELLHANDLE_CONVERT_(CellType) \
    void convert_(const CellType##Id & id, CellType##Handle<Geometry> & handle) { handle = vac_->cell(id); }

#define OPENVAC_DEFINE_CELLHANDLE_TO_CELLID_CONVERT_(CellType) \
    void convert_(const CellType##Handle<Geometry> & handle, CellType##Id & id) { id = handle->id(); }

namespace OpenVAC
{

template <class T, class U, class Geometry>
class CellDataConverter
{
public:
    void convert(const TCellData<T, Geometry> & fromData, TCellData<U, Geometry> & toData)
    {
        assert(fromData.type() == toData.type());
        CellType type = fromData.type();
        OPENVAC_FOREACH_FINAL_CELL_TYPE(OPENVAC_IF_TYPE_MATCHES_THEN_CALL_CONVERT_)
    }

    void convert(const TKeyVertexData<T, Geometry> & fromData, TKeyVertexData<U, Geometry> & toData)
    {
        toData.frame = fromData.frame;
        toData.geometry() = fromData.geometry();
    }

    void convert(const TKeyEdgeData<T, Geometry> & fromData, TKeyEdgeData<U, Geometry> & toData)
    {
        toData.frame = fromData.frame;
        convert_(fromData.startVertex, toData.startVertex);
        convert_(fromData.endVertex, toData.endVertex);
        toData.geometry() = fromData.geometry();
    }

    // XXX TODO other cell types

protected:
    OPENVAC_FOREACH_CELL_TYPE(OPENVAC_DECLARE_CONVERT_)
};

template <class Geometry>
class OpCellDataToCellDataConverter: public CellDataConverter<IdsAsRefs, HandlesAsRefs<Geometry>, Geometry>
{
public:
    OpCellDataToCellDataConverter(VAC<Geometry> * vac) : vac_(vac) {}

private:
    VAC<Geometry> * vac_;
    OPENVAC_FOREACH_CELL_TYPE(OPENVAC_DEFINE_CELLID_TO_CELLHANDLE_CONVERT_)
};

template <class Geometry>
class CellDataToOpCellDataConverter: public CellDataConverter<HandlesAsRefs<Geometry>, IdsAsRefs, Geometry>
{
public:
    CellDataToOpCellDataConverter(VAC<Geometry> * vac) : vac_(vac) {}

private:
    VAC<Geometry> * vac_;
    OPENVAC_FOREACH_CELL_TYPE(OPENVAC_DEFINE_CELLHANDLE_TO_CELLID_CONVERT_)
};

}

#endif
