// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_OPERATOR_H
#define OPENVAC_OPERATOR_H

#include <OpenVAC/Core/Memory.h>
#include <OpenVAC/Topology/CellHandle.h>
#include <OpenVAC/Operators/OpCellData.h>
#include <OpenVAC/Operators/CellDataConverter.h>

#include <vector>
#include <map>
#include <cassert>

#define OPENVAC_IF_TYPE_MATCHES_THEN_RETURN_MAKE_SHARED_(CellType_) \
    if (type == CellType::CellType_) \
    { \
        return std::make_shared<CellType_>(vac(), id); \
    }

// Convenient macros for derived operators. It defines an override of
// Operator::compute() and Operator::apply() to return a reference to an
// OpDerivedType instead of a reference to an Operator
#define OPENVAC_OPERATOR_OVERRIDE_COMPUTE_AND_APPLY_(OpName) \
    OpName & compute() { Operator::compute(); return *this;} \
    OpName & apply()   { Operator::apply();   return *this;}

//#define OPENVAC_OPERATOR_DECLARE_OPERATOR_TYPEDEF_ \
//    typedef OpenVAC::Operator<Geometry> Operator;

#define OPENVAC_OPERATOR_DECLARE_VAC_TYPEDEF_ \
    typedef OpenVAC::VAC<Geometry> VAC;

#define OPENVAC_OPERATOR_DECLARE_CELL_TYPEDEF_(CellType) \
    typedef OpenVAC::CellType<Geometry> CellType;

#define OPENVAC_OPERATOR_DECLARE_HANDLE_TYPEDEF_(CellType) \
    typedef OpenVAC::CellType##Handle<Geometry> CellType##Handle;

#define OPENVAC_OPERATOR_DECLARE_DATA_TYPEDEF_(CellType) \
    typedef OpenVAC::CellType##Data<Geometry> CellType##Data;

#define OPENVAC_OPERATOR_DECLARE_OP_DATA_TYPEDEF_(CellType) \
    typedef OpenVAC::Op##CellType##Data<Geometry> Op##CellType##Data;

#define OPENVAC_OPERATOR_DECLARE_OP_DATA_PTR_TYPEDEF_(CellType) \
    typedef OpenVAC::Op##CellType##DataPtr<Geometry> Op##CellType##DataPtr;

#define OPENVAC_OPERATOR_DECLARE_TYPEDEFS \
    OPENVAC_OPERATOR_DECLARE_VAC_TYPEDEF_ \
    OPENVAC_FOREACH_CELL_TYPE(OPENVAC_OPERATOR_DECLARE_HANDLE_TYPEDEF_) \
    OPENVAC_FOREACH_CELL_DATA_TYPE(OPENVAC_OPERATOR_DECLARE_DATA_TYPEDEF_) \
    OPENVAC_FOREACH_CELL_DATA_TYPE(OPENVAC_OPERATOR_DECLARE_OP_DATA_TYPEDEF_) \
    OPENVAC_FOREACH_CELL_DATA_TYPE(OPENVAC_OPERATOR_DECLARE_OP_DATA_PTR_TYPEDEF_)

#define OPENVAC_OPERATOR_USING_NEW_CELL_(CellType) \
    using Operator::new##CellType;

#define OPENVAC_OPERATOR_USING_BASE_METHODS_ \
    using Operator::vac; \
    using Operator::isValid; \
    using Operator::isComputed; \
    using Operator::isApplied; \
    using Operator::newCells; \
    using Operator::deletedCells; \
    OPENVAC_FOREACH_FINAL_CELL_TYPE(OPENVAC_OPERATOR_USING_NEW_CELL_)

#define OPENVAC_OPERATOR(OpName) \
    typedef OpenVAC::Operator<Geometry> Operator; \
    OPENVAC_OPERATOR_DECLARE_TYPEDEFS \
    OPENVAC_OPERATOR_USING_BASE_METHODS_ \
    OPENVAC_OPERATOR_OVERRIDE_COMPUTE_AND_APPLY_(OpName)

namespace OpenVAC
{

template <class Geometry> class VAC;

template <class Geometry>
class Operator
{
private:
    // Private typedefs
    typedef OpenVAC::CellSharedPtr<Geometry> CellSharedPtr;
    typedef OpenVAC::OpCellDataSharedPtr<Geometry> OpCellDataSharedPtr;
    typedef OpenVAC::OpCellDataToCellDataConverter<Geometry> OpCellDataToCellDataConverter;
    OPENVAC_FOREACH_CELL_TYPE(OPENVAC_OPERATOR_DECLARE_CELL_TYPEDEF_)

public:
    // Public typedefs
    OPENVAC_OPERATOR_DECLARE_TYPEDEFS

    // Constructor
    Operator(VAC * vac = nullptr) :
        vac_(vac),
        isValidated_(false),
        isComputed_(false),
        isApplied_(false),
        numIdRequested_(0) {}

    // Returns the VAC this operator is bound to.
    VAC * vac() const { return vac_; }

    // Checks whether the operation is valid.
    bool isValid()
    {
        if (!isValidated_)
        {
            valid_ = isValid_();
            isValidated_ = true;
        }
        return valid_;
    }

    // Computes operation. Does nothing if already computed.
    // Aborts if not valid. Returns this Operator.
    bool isComputed() const { return isComputed_; }
    Operator & compute()
    {
        assert(isValid());
        if (!isComputed_)
        {
            compute_();
            isComputed_ = true;
        }
        return *this;
    }


    // Computes operation if not computed yet, then applies operation to VAC.
    // Aborts if not valid or already applied. Returns this Operator.
    bool isApplied() const { return isApplied_; }
    Operator & apply()
    {
        assert(!isApplied());
        compute();
        apply_();
        isApplied_ = true;
        return *this;
    }

    // Get info about operation (compute and/or apply must have been called)
    const std::vector<CellId> & newCells() { return newCells_; }
    const std::vector<CellId> & deletedCells() { return deletedCells_; }

protected:
    // Methods that must be implemented by derived classes
    virtual bool isValid_()=0;
    virtual void compute_()=0;

    // Methods to be used by derived classes
    OpKeyVertexDataPtr newKeyVertex(KeyVertexId * outId = nullptr) { return newCell_<OpKeyVertexData>(outId); }
    OpKeyEdgeDataPtr   newKeyEdge  (KeyEdgeId *   outId = nullptr) { return newCell_<OpKeyEdgeData>(outId); }

private:
    VAC * vac_;

    // Validity
    bool isValidated_;
    bool valid_;

    // Computation
    bool isComputed_;
    std::map<CellId, OpCellDataSharedPtr> cellsBefore_;
    std::map<CellId, OpCellDataSharedPtr> cellsAfter_;
    std::vector<CellId> newCells_;
    std::vector<CellId> deletedCells_;

    // Application
    bool isApplied_;
    void apply_()
    {
        // Deallocate deleted cells
        for (CellId id: deletedCells_)
        {
            vac_->cellManager_.remove(id);
        }

        // Allocate new cells
        for (CellId id: newCells_)
        {
            CellSharedPtr cell = make_shared(cellsAfter_[id]->type(), id);
            vac_->cellManager_.insert(id, cell);
        }

        // Copy cell data from operator to VAC
        OpCellDataToCellDataConverter converter(vac());
        for (auto & id_data_pair: cellsAfter_)
        {
            CellId id = id_data_pair.first;
            const OpCellData & opCellData = *id_data_pair.second;
            CellData & cellData = vac_->cellManager_[id]->data();

            converter.convert(opCellData, cellData);
        }
    }

    // Keep track of requested IDs
    mutable unsigned int numIdRequested_;

    // Private methods
    template <class OpCellDataType>
    WeakPtr<OpCellDataType> newCell_(CellId * outId)
    {
        // Get available ID
        CellId id = getAvailableId_();
        if (outId)
            *outId = id;

        // Allocate OpCellData
        auto opCellData = std::make_shared<OpCellDataType>();

        // Insert in cellsAfter_
        auto res = cellsAfter_.insert(std::make_pair(id, opCellData));

        // Abort if ID already taken
        assert(res.second);

        // Tag this ID as a cell created by the operator
        newCells_.push_back(id);

        // Return the allocated OpCellData
        return opCellData;
    }

    CellId getAvailableId_() const
    {
        // Note: calling vac_->cellManager_.getAvailableId() repeatedly would always
        // give the same ID (since we're not inserting a cell after the call).
        // Therefore, for the second call to Operator::getAvailableId(), we actually
        // need to request two IDs, and ignore the first one. The IDs are guaranteed
        // to be always generated deterministically in the same order.
        ++numIdRequested_;
        std::vector<CellId> ids = vac_->cellManager_.getAvailableIds(numIdRequested_);
        return ids[numIdRequested_-1];
    }

    std::vector<CellId> getAvailableIds_(unsigned int numIds) const
    {
        // Same as above: we re-generate previously generated IDs, but only return
        // the new ones
        numIdRequested_ += numIds;
        std::vector<CellId> ids = vac_->cellManager_.getAvailableIds(numIdRequested_);
        return std::vector<CellId>(&ids[numIdRequested_-numIds], &ids[numIdRequested_]);
    }

    CellSharedPtr make_shared(CellType type, CellId id) const
    {
        OPENVAC_FOREACH_FINAL_CELL_TYPE(OPENVAC_IF_TYPE_MATCHES_THEN_RETURN_MAKE_SHARED_)
    }
};

}

#endif // OPENVAC_OPERATOR_H
