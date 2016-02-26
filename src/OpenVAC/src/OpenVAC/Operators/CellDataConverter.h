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

namespace OpenVAC
{

template <class T, class U>
class CellDataConverter
{
public:
    void convert(const TCellData<T> & fromData, TCellData<U> & toData)
    {
        assert(fromData.type() == toData.type());
        CellType type = fromData.type();
        if (type == CellType::KeyVertex)
            convert(*fromData.toKeyVertexData(), *toData.toKeyVertexData());
        else if (type == CellType::KeyVertex)
            convert(*fromData.toKeyEdgeData(), *toData.toKeyEdgeData());
        else
        {
            // XXX TODO
        }
    }

    void convert(const TKeyVertexData<T> & fromData, TKeyVertexData<U> & toData)
    {
        toData.frame = fromData.frame;
    }

    void convert(const TKeyEdgeData<T> & fromData, TKeyEdgeData<U> & toData)
    {
        toData.frame = fromData.frame;
        convert_(fromData.startVertex, toData.startVertex);
        convert_(fromData.endVertex, toData.endVertex);
    }

    void convert(const TKeyFaceData<T> & fromData, TKeyFaceData<U> & toData)
    {
        // XXX TODO
    }

    void convert(const TInbetweenVertexData<T> & fromData, TInbetweenVertexData<U> & toData)
    {
        // XXX TODO
    }

    void convert(const TInbetweenEdgeData<T> & fromData, TInbetweenEdgeData<U> & toData)
    {
        // XXX TODO
    }

    void convert(const TInbetweenFaceData<T> & fromData, TInbetweenFaceData<U> & toData)
    {
        // XXX TODO
    }

protected:

#define OPENVAC_DECLARE_VIRTUAL_CELL_DATA_CONVERT(CellType) \
    virtual void convert_(const typename T::CellType##Ref & fromRef, typename U::CellType##Ref & toRef)=0;

    OPENVAC_DECLARE_VIRTUAL_CELL_DATA_CONVERT(Cell)
    OPENVAC_DECLARE_VIRTUAL_CELL_DATA_CONVERT(KeyCell)
    //OPENVAC_DECLARE_VIRTUAL_CELL_DATA_CONVERT(InbetweenCell) // XXX TODO
    OPENVAC_DECLARE_VIRTUAL_CELL_DATA_CONVERT(VertexCell)
    OPENVAC_DECLARE_VIRTUAL_CELL_DATA_CONVERT(EdgeCell)
    //OPENVAC_DECLARE_VIRTUAL_CELL_DATA_CONVERT(FaceCell)
    OPENVAC_DECLARE_VIRTUAL_CELL_DATA_CONVERT(KeyVertex)
    OPENVAC_DECLARE_VIRTUAL_CELL_DATA_CONVERT(KeyEdge)
    //OPENVAC_DECLARE_VIRTUAL_CELL_DATA_CONVERT(KeyFace)
    //OPENVAC_DECLARE_VIRTUAL_CELL_DATA_CONVERT(InbetweenVertex)
    //OPENVAC_DECLARE_VIRTUAL_CELL_DATA_CONVERT(InbetweenEdge)
    //OPENVAC_DECLARE_VIRTUAL_CELL_DATA_CONVERT(InbetweenFace)
};

class OpCellDataToCellDataConverter: public CellDataConverter<OpCellDataTrait, CellDataTrait>
{
public:
    OpCellDataToCellDataConverter(VAC * vac) : vac_(vac) {}

private:
    VAC * vac_;

#define OPENVAC_DEFINE_CELLID_TO_CELLHANDLE_CONVERT(CellType) \
    void convert_(const CellType##Id & id, CellType##Handle & handle) { handle = vac_->cell(id); }

    OPENVAC_DEFINE_CELLID_TO_CELLHANDLE_CONVERT(Cell)
    OPENVAC_DEFINE_CELLID_TO_CELLHANDLE_CONVERT(KeyCell)
    //OPENVAC_DEFINE_CELLID_TO_CELLHANDLE_CONVERT(InbetweenCell) XXX TODO
    OPENVAC_DEFINE_CELLID_TO_CELLHANDLE_CONVERT(VertexCell)
    OPENVAC_DEFINE_CELLID_TO_CELLHANDLE_CONVERT(EdgeCell)
    //OPENVAC_DEFINE_CELLID_TO_CELLHANDLE_CONVERT(FaceCell)
    OPENVAC_DEFINE_CELLID_TO_CELLHANDLE_CONVERT(KeyVertex)
    OPENVAC_DEFINE_CELLID_TO_CELLHANDLE_CONVERT(KeyEdge)
    //OPENVAC_DEFINE_CELLID_TO_CELLHANDLE_CONVERT(KeyFace)
    //OPENVAC_DEFINE_CELLID_TO_CELLHANDLE_CONVERT(InbetweenVertex)
    //OPENVAC_DEFINE_CELLID_TO_CELLHANDLE_CONVERT(InbetweenEdge)
   // OPENVAC_DEFINE_CELLID_TO_CELLHANDLE_CONVERT(InbetweenFace)
};

class CellDataToOpCellDataConverter: public CellDataConverter<CellDataTrait, OpCellDataTrait>
{
public:
    CellDataToOpCellDataConverter(VAC * vac) : vac_(vac) {}

private:
    VAC * vac_;

#define OPENVAC_DEFINE_CELLHANDLE_TO_CELLID_CONVERT(CellType) \
    void convert_(const CellType##Handle & handle, CellType##Id & id) { id = handle->id(); }

    OPENVAC_DEFINE_CELLHANDLE_TO_CELLID_CONVERT(Cell)
    OPENVAC_DEFINE_CELLHANDLE_TO_CELLID_CONVERT(KeyCell)
    //OPENVAC_DEFINE_CELLHANDLE_TO_CELLID_CONVERT(InbetweenCell) XXX TODO
    OPENVAC_DEFINE_CELLHANDLE_TO_CELLID_CONVERT(VertexCell)
    OPENVAC_DEFINE_CELLHANDLE_TO_CELLID_CONVERT(EdgeCell)
    //OPENVAC_DEFINE_CELLHANDLE_TO_CELLID_CONVERT(FaceCell)
    OPENVAC_DEFINE_CELLHANDLE_TO_CELLID_CONVERT(KeyVertex)
    OPENVAC_DEFINE_CELLHANDLE_TO_CELLID_CONVERT(KeyEdge)
    //OPENVAC_DEFINE_CELLHANDLE_TO_CELLID_CONVERT(KeyFace)
    //OPENVAC_DEFINE_CELLHANDLE_TO_CELLID_CONVERT(InbetweenVertex)
    //OPENVAC_DEFINE_CELLHANDLE_TO_CELLID_CONVERT(InbetweenEdge)
    //OPENVAC_DEFINE_CELLHANDLE_TO_CELLID_CONVERT(InbetweenFace)
};

}

#endif
