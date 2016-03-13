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
#include <OpenVac/Core/TypeTraits.h>
#include <OpenVac/Data/UsingData.h>
#include <OpenVac/Geometry/UsingGeometry.h>
#include <OpenVac/Operators/CellDataCopier.h>

#include <vector>
#include <map>
#include <cassert>


/**************** Private macros to declare type aliases  ********************/

// Type aliases common to Operator and derived class

#define OPENVAC_OPERATOR_USING_GEOMETRY_ \
    using geometry_type = Geometry; \
    OPENVAC_USING_GEOMETRY(/* No prefix */, Geometry)

#define OPENVAC_OPERATOR_USING_VAC_ \
    using Vac = OpenVac::Vac<Geometry>;

#define OPENVAC_OPERATOR_USING_DATA_ \
    OPENVAC_USING_DATA(/*  No prefix  */, UsingCellHandlesAsCellRefs<Geometry>, Geometry)

#define OPENVAC_OPERATOR_USING_OP_DATA_ \
    OPENVAC_USING_DATA(/* Prefix = */ Op, UsingCellIdsAsCellRefs, Geometry)

#define OPENVAC_OPERATOR_USING_CELL_HANDLE_(CellType) \
    using CellType##Handle = Handle< OpenVac::CellType<Geometry> >;

#define OPENVAC_OPERATOR_DECLARE_TYPE_ALIASES_COMMON_ \
    OPENVAC_OPERATOR_USING_GEOMETRY_ \
    OPENVAC_OPERATOR_USING_VAC_ \
    OPENVAC_OPERATOR_USING_DATA_ \
    OPENVAC_OPERATOR_USING_OP_DATA_ \
    OPENVAC_FOREACH_CELL_TYPE(OPENVAC_OPERATOR_USING_CELL_HANDLE_)

// Type aliases specific to Operator base class

#define OPENVAC_OPERATOR_USING_CELL_(CellType) \
    using CellType = OpenVac::CellType<Geometry>;

#define OPENVAC_OPERATOR_DECLARE_TYPE_ALIASES_BASE_ONLY_ \
    OPENVAC_FOREACH_CELL_TYPE(OPENVAC_OPERATOR_USING_CELL_) \
    using CellSharedPtr = SharedPtr< Cell >; \
    using OpToCellDataCopier = OpenVac::OpToCellDataCopier<Geometry>; \
    using CellToOpDataCopier = OpenVac::CellToOpDataCopier<Geometry>;

// Type aliases specific to derived classes

#define OPENVAC_OPERATOR_DECLARE_OPERATOR_TYPEDEF_ \
    using Operator = OpenVac::Operator<Geometry>;

#define OPENVAC_OPERATOR_DECLARE_TYPE_ALIASES_DERIVED_ONLY_ \
    OPENVAC_OPERATOR_DECLARE_OPERATOR_TYPEDEF_


/****** Private macros to introduce Operator methods to derived classes  *****/

#define OPENVAC_OPERATOR_USING_NEW_CELL_(CellType) \
    using Operator::new##CellType;

#define OPENVAC_OPERATOR_USING_GET_CELL_(CellType) \
    using Operator::get##CellType;

#define OPENVAC_OPERATOR_USING_INHERITED_METHODS_ \
    using Operator::canBeApplied; \
    using Operator::newCells; \
    using Operator::deletedCells; \
    OPENVAC_FOREACH_FINAL_CELL_TYPE(OPENVAC_OPERATOR_USING_NEW_CELL_) \
    OPENVAC_FOREACH_FINAL_CELL_TYPE(OPENVAC_OPERATOR_USING_GET_CELL_)


/**** Private macro to override Operator::compute() and Operator::apply()  ***/

#define OPENVAC_OPERATOR_OVERRIDE_COMPUTE_AND_APPLY_(OpName) \
    OpName & compute(const Vac & vac) { Operator::compute(vac); return *this; } \
    OpName & apply(Vac & vac)         { Operator::apply(vac);   return *this; }


/********* Public macro that must appear in Operator derived classes *********/

/// The OPENVAC_OPERATOR(OpName) macro must appear in the private section of
/// classes inheriting Operator. It does the following:
///
///   * It declares type aliases for relevant OpenVac classes (e.g., 'using
///     VacPtr = OpenVac::WeakPtr<Vac<Geometry>>;'). This allows to write types
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
///     member functions, to change its return type from 'Operator &' to
///     the derived type 'OpName &'
///
/// Note: the template parameter of OpName must be called Geometry, otherwise
/// the macro won't work.
///
#define OPENVAC_OPERATOR(OpName) \
    private: \
        OPENVAC_OPERATOR_DECLARE_TYPE_ALIASES_COMMON_ \
        OPENVAC_OPERATOR_DECLARE_TYPE_ALIASES_DERIVED_ONLY_ \
    public: \
        OPENVAC_OPERATOR_OVERRIDE_COMPUTE_AND_APPLY_(OpName) \
        OPENVAC_OPERATOR_USING_INHERITED_METHODS_ \
    private:

namespace OpenVac
{


/*************************** UsingCellIdsAsCellRefs  *************************/

/// \class UsingCellIdsAsCellRefs OpenVac/Operators/Operator.h
///
/// A class that declares CellType##Ref as an alias for CellType##Id, for
/// each cell type. It is used as the T template argument of the Data classes
/// used within the Operator classes.
///
class UsingCellIdsAsCellRefs
{
#define OPENVAC_OPERATOR_USING_ID_AS_REF_(CellType) \
    using CellType##Ref = CellType##Id;

public:
    OPENVAC_FOREACH_CELL_TYPE(OPENVAC_OPERATOR_USING_ID_AS_REF_)
};


/********************************* Operator  *********************************/

template <class Geometry> class Vac;

/// \class Operator OpenVac/Operators/Operator.h
/// \brief The base class for all operators
///
///
template <class Geometry>
class Operator
{
private:
    // Type aliases
    OPENVAC_OPERATOR_DECLARE_TYPE_ALIASES_COMMON_
    OPENVAC_OPERATOR_DECLARE_TYPE_ALIASES_BASE_ONLY_

public:
    /// Constructs an Operator.
    ///
    /// Note: the constructors of derived classes must only take Cell IDs as
    /// parameters. They shouldn't take a Vac, or Handles. This ensures that
    /// any access of data in a Vac is done through Operator, and therefore
    /// that Operator is aware of it and can later check that you didn't mess
    /// up that data.
    ///
    Operator() :
        vac_(nullptr),
        canBeComputed_(true),
        canBeApplied_(false),
        numIdRequested_(0)
    {
        // Note: we can't call compute() directly in this constructor,
        // before it calls virtual methods that won't be dispatched correctly.
        // But that's actually a good thing: with a "auto-compute on construction"
        // semantics, the client would have to pass a Vac as argument, and the
        // constructor would be tempted to use it, breaking the design that all
        // access to the actual Vac should be done indirectly via the base class
        // that ensures to do the right thing.
    }

    /// Determines whether it is legal to apply the operator to the given \p
    /// vac, and if yes computes and caches all the data required to apply the
    /// operator.
    ///
    /// Aborts if compute() has already been called on this instance. If you
    /// wish to compute the same operation (i.e., same input cell IDs that
    /// define the operation) to different VACs (the result of the computation
    /// may be different, one obvious reason being that IDs assigned to new
    /// cells would differ), you need to first create the Operator, make copies
    /// of it, and only then call compute() on each of them.
    ///
    /// This function is re-entrant, but obviously not thread-safe as it
    /// can only be call once anyway.
    ///
    // XXX TODO: provide a clear() method to allow recomputation.
    // this must call a virtual clear_ as well.
    Operator & compute(const Vac & vac)
    {
        // XXX TODO acquire mutex here
        assert(canBeComputed_);
        canBeComputed_ = false;
        // XXX TODO release mutex here

        vac_ = &vac;
        canBeApplied_ = compute_();
        vac_ = nullptr;

        return *this;
    }

    /// Returns true if compute() has been called and has determined that it
    /// was legal to apply this operator to the given \p vac.
    ///
    /// \sa operator bool()
    ///
    bool canBeApplied() const { return canBeApplied_; }

    /// Casts to a boolean whose value is equal to canBeApplied(). This method
    /// is provided to allow the convenient syntax:
    ///
    /// \code
    /// if (auto op = Operators::MakeKeyEdge(v1,v2).compute(vac))
    ///     op.apply(
    ///
    ///
    /// \sa canBeApplied()
    ///
    explicit operator bool() const { return canBeApplied(); }

    /// Applies the computed operator to the given \p vac. The given \p vac
    /// must be identical to the \p vac provided in the compute() function
    /// (i.e., same number of cells, same cell IDs, same topology, same
    /// geometry), but it can be a different instance, or may have been modified and
    /// come back to the same state (e.g., undo followed by redo). Otherwise, the
    /// behavior is undefined.
    ///
    /// This function is reentrant and thread-safe: two threads may
    /// concurrently call the apply() function of a shared Operator instance to
    /// two (identical) Vac instances.
    ///
    Operator & apply(Vac & vac)
    {
        assert(canBeApplied_);

        // Deallocate deleted cells
        for (CellId id: deletedCells_)
        {
            vac.cellManager_.remove(id);
        }

        // Allocate new cells
        for (CellId id: newCells_)
        {
            // Allocate cell
            auto & data = cellsAfter_.at(id);
            CellSharedPtr cell = make_shared_cell_(&vac, data->type(), id);

            // Insert in ID Manager
            vac.cellManager_.insert(id, cell);
        }

        // Copy cell data from operator to VAC
        OpToCellDataCopier copier(&vac);
        for (auto & id_data_pair: cellsAfter_)
        {
            // Get ID and new data of cell
            CellId id = id_data_pair.first;
            const OpCellData & opCellData = *id_data_pair.second;

            // Get handle of corresponding cell in VAC
            CellHandle toCell = vac.cellManager_[id]; // XXX avoidable handle copy here?

            // Copy cell data from operator to VAC
            CellData & cellData = toCell->data();
            copier.copy(opCellData, cellData);
        }


        return *this;
    }

    /// Returns the IDs of the cells that are created by this operator. This
    /// must be called after compute(), but may be called before apply().
    /// Returns an empty vector if not yet computed or cannot be applied.
    ///
    const std::vector<CellId> & newCells() { return newCells_; }

    /// Returns the IDs of the cells that are destructed by this operator. This
    /// must be called after compute(), but may be called before apply().
    /// Returns an empty vector if not yet computed or cannot be applied.
    ///
    const std::vector<CellId> & deletedCells() { return deletedCells_; }

protected:

    /// Actual implementation of the operator. This method is called by
    /// compute(), and must be implemented by derived classes. It must
    /// return whether or not the operation is legal.
    ///
    virtual bool compute_()=0;

    // Methods to be used by derived classes
    OpKeyVertexData * newKeyVertex(KeyVertexId * outId = nullptr) { return newCell_<OpKeyVertexData>(outId); }
    OpKeyEdgeData   * newKeyEdge  (KeyEdgeId *   outId = nullptr) { return newCell_<OpKeyEdgeData>(outId); }

    OpKeyVertexData * getKeyVertex(KeyVertexId id)
    {
        OpCellData * data = getCell_(id);
        assert(data->type() == CellType::KeyVertex);
        return static_cast<OpKeyVertexData *>(data);
    }

    OpKeyEdgeData * getKeyEdge(KeyEdgeId id)
    {
        OpCellData * data = getCell_(id);
        assert(data->type() == CellType::KeyEdge);
        return static_cast<OpKeyEdgeData *>(data);
    }

    OpCellData * getCell_(CellId id)
    {
        OpCellData * res = nullptr;

        // Is the data already in cellsAfter_?
        auto it = cellsAfter_.find(id);
        if (it != cellsAfter_.end())
            return it->second.get();

        // If not, then it musn't be in cellsBefore_ (that would
        // mean the compute_() function called deleteCell(), which
        // is the only way to have a cell ID in cellBefore_ but not
        // in cellAfter_
        assert(cellsBefore_.find(id) == cellsBefore_.end());

        // And it must be in the VAC, otherwise the ID exists nowhere!
        assert(vac_->cellManager_.contains(id)); // XXX can be optimized. Two lookups instead of one.

        // Get cell data
        CellHandle toCell = vac_->cellManager_[id]; // XXX avoidable handle copy here?
        const CellData & cellData = toCell->data();

        // Make op cell data before and after
        CellType type = cellData.type();
        OpCellData * opCellDataBefore =
                new_cell_data<UsingCellIdsAsCellRefs, Geometry>(type);
        OpCellData * opCellDataAfter =
                new_cell_data<UsingCellIdsAsCellRefs, Geometry>(type);

        // Make into unique pointers
        auto opCellDataBefore_unique_ptr =
                std::unique_ptr<OpCellData>(opCellDataBefore);
        auto opCellDataAfter_unique_ptr =
                std::unique_ptr<OpCellData>(opCellDataAfter);

        // Move into operator maps
        auto resBefore = cellsBefore_.insert(std::make_pair(
                              id, std::move(opCellDataBefore_unique_ptr)));
        auto resAfter = cellsAfter_.insert(std::make_pair(
                              id, std::move(opCellDataAfter_unique_ptr)));

        // Abort if ID already taken
        assert(resBefore.second);
        assert(resAfter.second);

        // Copy data from cell data to op cell data
        cellToOpDataCopier_.copy(cellData, *opCellDataBefore);
        cellToOpDataCopier_.copy(cellData, *opCellDataAfter);

        // Returns the relevant op cell data
        return opCellDataAfter;
    }

private:
    // Temp member variable (to hide it from derived classes).
    // It is always equal to nullptr, except during the execution
    // of compute(vac), where it is equal to the vac argument
    const Vac * vac_;

    // Computation
    bool canBeComputed_;
    bool canBeApplied_;
    std::map<CellId, std::unique_ptr<OpCellData>> cellsBefore_;
    std::map<CellId, std::unique_ptr<OpCellData>> cellsAfter_;
    std::vector<CellId> newCells_;
    std::vector<CellId> deletedCells_;

    // VAC to Operator cell copier
    CellToOpDataCopier cellToOpDataCopier_;

    // Keep track of requested IDs
    mutable unsigned int numIdRequested_;

    // Private methods
    template <class OpCellDataType>
    OpCellDataType * newCell_(CellId * outId)
    {
        // Get available ID
        CellId id = getAvailableId_();
        if (outId)
            *outId = id;

        // Allocate OpCellData
        OpCellDataType * data = new OpCellDataType();
        auto data_unique_ptr = std::unique_ptr<OpCellDataType>(data);

        // Insert in cellsAfter_
        auto res = cellsAfter_.insert(std::make_pair(id, std::move(data_unique_ptr)));

        // Abort if ID already taken
        assert(res.second);

        // Tag this ID as a cell created by the operator
        newCells_.push_back(id);

        // Return the allocated OpCellData
        return data;
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

    CellSharedPtr make_shared_cell_(Vac * vac, CellType type, CellId id) const
    {
        #define OPENVAC_OPERATOR_RETURN_MAKE_SHARED_IF_TYPE_MATCHES_(CellType_) \
            if (type == CellType::CellType_) \
                return std::make_shared<CellType_>(vac, id);

        OPENVAC_FOREACH_FINAL_CELL_TYPE(
                    OPENVAC_OPERATOR_RETURN_MAKE_SHARED_IF_TYPE_MATCHES_)
    }
};

/// \addtogroup Operators
/// @{

/// \namespace OpenVac::Operators
/// \brief Free functions that construct or apply VAC operators.
namespace Operators
{
}

/// @} end addtogroup Operators

}

#endif // OPENVAC_OPERATOR_H
