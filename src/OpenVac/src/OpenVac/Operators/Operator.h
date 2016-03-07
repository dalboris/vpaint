// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_OPERATOR_H
#define OPENVAC_OPERATOR_H

#include <OpenVac/Core/Memory.h>
#include <OpenVac/Core/GeometryType.h>
#include <OpenVac/Topology/CellHandle.h>
#include <OpenVac/Operators/OpCellData.h>
#include <OpenVac/Operators/CellDataConverter.h>

#include <vector>
#include <map>
#include <cassert>

#define OPENVAC_OPERATOR_RETURN_MAKE_SHARED_IF_TYPE_MATCHES_(CellType_) \
    if (type == CellType::CellType_) \
        return std::make_shared<CellType_>(vac(), id);

#define OPENVAC_OPERATOR_OVERRIDE_COMPUTE_AND_APPLY_(OpName) \
    OpName & compute() { Operator::compute(); return *this;} \
    OpName & apply()   { Operator::apply();   return *this;}

#define OPENVAC_OPERATOR_DECLARE_OPERATOR_TYPEDEF_ \
    typedef OpenVac::Operator<Geometry> Operator;

#define OPENVAC_OPERATOR_DECLARE_VAC_TYPEDEF_ \
    typedef OpenVac::Vac<Geometry> Vac; \
    typedef OpenVac::WeakPtr<Vac> VacPtr;

#define OPENVAC_OPERATOR_DECLARE_CELL_TYPEDEF_(CellType) \
    typedef OpenVac::CellType<Geometry> CellType;

#define OPENVAC_OPERATOR_DECLARE_HANDLE_TYPEDEF_(CellType) \
    typedef OpenVac::CellType##Handle<Geometry> CellType##Handle;

#define OPENVAC_OPERATOR_DECLARE_DATA_TYPEDEF_(CellType) \
    typedef OpenVac::CellType##Data<Geometry> CellType##Data;

#define OPENVAC_OPERATOR_DECLARE_OP_DATA_TYPEDEF_(CellType) \
    typedef OpenVac::Op##CellType##Data<Geometry> Op##CellType##Data;

#define OPENVAC_OPERATOR_DECLARE_OP_DATA_PTR_TYPEDEF_(CellType) \
    typedef OpenVac::Op##CellType##DataPtr<Geometry> Op##CellType##DataPtr;

#define OPENVAC_OPERATOR_USING_NEW_CELL_(CellType) \
    using Operator::new##CellType;

#define OPENVAC_OPERATOR_USING_INHERITED_METHODS_ \
    using Operator::vac; \
    using Operator::isValid; \
    using Operator::isComputed; \
    using Operator::isApplied; \
    using Operator::newCells; \
    using Operator::deletedCells; \
    OPENVAC_FOREACH_FINAL_CELL_TYPE(OPENVAC_OPERATOR_USING_NEW_CELL_)

/// The OPENVAC_OPERATOR(OpName) declares typedefs for relevant OpenVac classes
/// (e.g., 'typedef OpenVac::WeakPtr<Vac<Geometry>> VacPtr;'). This allows to
/// write types such as VacPtr and KeyVertexHandle without specifying any
/// template parameter, which makes the code more readable.
///
/// Note: the template parameter of OpName must be called Geometry, otherwise
/// the macro won't work.

#define OPENVAC_OPERATOR_DECLARE_TYPEDEFS \
    OPENVAC_OPERATOR_DECLARE_VAC_TYPEDEF_ \
    OPENVAC_FOREACH_CELL_TYPE(OPENVAC_OPERATOR_DECLARE_HANDLE_TYPEDEF_) \
    OPENVAC_FOREACH_CELL_DATA_TYPE(OPENVAC_OPERATOR_DECLARE_DATA_TYPEDEF_) \
    OPENVAC_FOREACH_CELL_DATA_TYPE(OPENVAC_OPERATOR_DECLARE_OP_DATA_TYPEDEF_) \
    OPENVAC_FOREACH_CELL_DATA_TYPE(OPENVAC_OPERATOR_DECLARE_OP_DATA_PTR_TYPEDEF_)

/// The OPENVAC_OPERATOR(OpName) macro does the following:
///
///   * It declares typedefs for relevant OpenVac classes (e.g., 'typedef
///     OpenVac::WeakPtr<Vac<Geometry>> VacPtr;'). This allows to write types
///     such as Operator, VacPtr, and KeyVertexHandle without specifying any
///     template parameter, which makes the code more readable.
///
///   * It explicitely introduces member functions inherited from the Operator
///     to the derived class definition, via using declarations (e.g., 'using
///     Operator::isComputed;'). This allows to write function calls such as
///     isComputed() instead of this->isComputed(), which would otherwise be
///     necessary due to the nondependent name lookup rule of class templates
///     (e.g., see <a href="https://isocpp.org/wiki/faq/templates#nondependent-name-lookup-members">
///     this C++ FAQ entry</a>). This makes the code more readable.
///
///   * Finally, it overrides the Operator::compute() and Operator::apply()
///     member functions, to change its return type from the 'Operator &' to
///     the derived class 'OpName &'
///
/// Note: the template parameter of OpName must be called Geometry, otherwise
/// the macro won't work.

#define OPENVAC_OPERATOR(OpName) \
    OPENVAC_OPERATOR_DECLARE_TYPEDEFS \
    OPENVAC_OPERATOR_DECLARE_OPERATOR_TYPEDEF_ \
    OPENVAC_OPERATOR_USING_INHERITED_METHODS_ \
    OPENVAC_OPERATOR_OVERRIDE_COMPUTE_AND_APPLY_(OpName)

namespace OpenVac
{

template <class Geometry> class Vac;

template <class Geometry>
class Operator
{
private:
    // Private typedefs
    typedef OpenVac::CellSharedPtr<Geometry> CellSharedPtr;
    typedef OpenVac::OpCellDataSharedPtr<Geometry> OpCellDataSharedPtr;
    typedef OpenVac::OpCellDataToCellDataConverter<Geometry> OpCellDataToCellDataConverter;
    OPENVAC_FOREACH_CELL_TYPE(OPENVAC_OPERATOR_DECLARE_CELL_TYPEDEF_)

public:
    // Public typedefs
    OPENVAC_OPERATOR_DECLARE_TYPEDEFS

    // Constructor
    Operator(VacPtr vac = VacPtr()) :
        vac_(vac),
        isValidated_(false),
        isComputed_(false),
        isApplied_(false),
        numIdRequested_(0) {}

    // Returns the VAC this operator is bound to.
    VacPtr vac() const { return vac_; }

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
    VacPtr vac_;

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
            CellHandle toCell = vac_->cellManager_[id];
            CellData & cellData = toCell->data();
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
        OPENVAC_FOREACH_FINAL_CELL_TYPE(
                    OPENVAC_OPERATOR_RETURN_MAKE_SHARED_IF_TYPE_MATCHES_)
    }
};

/// \addtogroup Operators
/// @{

/// \namespace OpenVac::Operators
/// \brief Free functions to construct or apply Vac operators
namespace Operators
{
}

/// @} Doxygen Operators group


}

#endif // OPENVAC_OPERATOR_H
