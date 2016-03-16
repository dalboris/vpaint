// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_OPERATOR_H
#define OPENVAC_OPERATOR_H

#include <OpenVac/Core/ForeachCellType.h>
#include <OpenVac/Core/CellId.h>
#include <OpenVac/Data/CellData.h>
#include <OpenVac/Operators/Util/Ids.h>
#include <OpenVac/Operators/Util/IdsToHandlesCopier.h>
#include <OpenVac/Operators/Util/HandlesToIdsCopier.h>
#include <OpenVac/Vac.h>

#include <vector>
#include <map>
#include <cassert>

namespace OpenVac
{

class Vac;

/// \class Operator OpenVac/Operators/Operator.h
/// \brief The base class for all operators
///
/// Note: methods of derived classes shouldn't take a Vac or Handles as
/// parameters, and should instead use cell IDs. This ensures that any access
/// of data in a Vac is done through the Operator base class, which is then
/// aware of it and keep track what cells the derived class has touched.
///
class Operator
{
public:
    /// Constructs an Operator.
    ///
    Operator() :
        vac_(nullptr),
        canBeComputed_(true),
        canBeApplied_(false),
        numIdRequested_(0)
    {
        // Note: we can't auto-call compute() in the constructor, since it
        // calls virtual methods that won't be dispatched correctly, thus
        // an "auto-compute on construction" semantics cannot be implemented.
        // Though, that may be a good thing: if such semantics were
        // implemented, then the client would have to pass a Vac as argument of
        // the derived constructor, breaking the rule that methods of derived
        // classes shouldn't take directly Vac or Handles as parameters.
    }

    /// Determines whether it is allowed to apply this operator to the given \p
    /// vac, and if yes computes and caches all the data required to apply the
    /// operator. Returns this Operator to allow function chaining.
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
    Operator & compute(const Vac & vac)
    {
        assert(canBeComputed_);
        canBeComputed_ = false;

        vac_ = &vac;
        canBeApplied_ = compute_();
        vac_ = nullptr;

        return *this;
    }

    /// Returns true if compute() has been called and has determined that it
    /// was allowed to apply this operator to the given \p vac.
    ///
    /// \sa operator bool()
    ///
    bool canBeApplied() const { return canBeApplied_; }

    /// Casts to a boolean whose value is equal to canBeApplied().
    ///
    /// \sa canBeApplied()
    ///
    explicit operator bool() const { return canBeApplied(); }

    /// Applies the computed operator to the given \p vac. The given \p vac
    /// must be identical to the \p vac provided in the compute() function
    /// (i.e., same number of cells, same cell IDs, same topology, same
    /// geometry), but it can be a different instance, or may have been
    /// modified and come back to the same state (e.g., undo followed by redo).
    /// Otherwise, the behavior is undefined. Returns this Operator to allow
    /// function chaining.
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
            // Remove cell from ID Manager
            vac.cellManager_.remove(id);
        }

        // Allocate new cells
        for (CellId id: newCells_)
        {
            // Allocate cell
            UniquePtr<CellData<Ids>> & data = cellsAfter_.at(id);
            SharedPtr<Cell> cell = makeCell_(&vac, data->type(), id);

            // Insert cell to ID Manager
            vac.cellManager_.insert(id, cell);
        }

        // Copy cell data from Operator to Vac
        IdsToHandlesCopier copier(&vac);
        for (auto & entry: cellsAfter_)
        {
            // Get Id and CellData stored in Operator
            CellId id = entry.first;
            const CellData<Ids> & opCellData = *entry.second;

            // Get reference to CellData stored in Vac
            SharedPtr<Cell> & cell = vac.cellManager_[id];
            CellData<Handles> & vacCellData = cell->data();

            // Copy cell data from operator to Vac
            copier.copy(opCellData, vacCellData);
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
    /// return whether or not the operation is allowed.
    ///
    virtual bool compute_()=0;

    #define OPENVAC_OPERATOR_NEW_CELL_(CellType) \
        /** Returns the cell data of a new CellType to be created by the */ \
        /** Operator. This method is provided for use in the subclass    */ \
        /** implementations of compute_().                               */ \
        CellType##Data<Ids> * new##CellType(CellType##Id * outId = nullptr) \
        { \
            return newCell_<CellType##Data<Ids>>(outId); \
        }

    OPENVAC_FOREACH_FINAL_CELL_TYPE(OPENVAC_OPERATOR_NEW_CELL_)

    #define OPENVAC_OPERATOR_GET_CELL_(CellType) \
        /** Returns the cell data of an existing CellType to be modified  */ \
        /** by the Operator. Returns nullptr if no cell with the given ID */ \
        /** exists, or if it exists but is not of the requested type.     */ \
    /** Aborts if the requested cell has been deleted by the Operator */ \
    /** subclass. */ \
        /** This method is provided for use in the subclass               */ \
        /** implementations of compute_().                                */ \
        CellType##Data<Ids> * get##CellType(CellType##Id outId) \
        { \
            return getCell_<CellType##Data<Ids>>(outId); \
        }

    OPENVAC_FOREACH_FINAL_CELL_TYPE(OPENVAC_OPERATOR_GET_CELL_)


private:
    // Temp member variable (to hide it from derived classes).
    // It is always equal to nullptr, except during the execution
    // of compute(vac), where it is equal to the vac argument
    const Vac * vac_;

    // Computation
    bool canBeComputed_;
    bool canBeApplied_;
    std::map<CellId, UniquePtr<CellData<Ids>>> cellsBefore_;
    std::map<CellId, UniquePtr<CellData<Ids>>> cellsAfter_;
    std::vector<CellId> newCells_;
    std::vector<CellId> deletedCells_;

    // VAC to Operator cell copier
    HandlesToIdsCopier handlesToIdsCopier_;

    // Keep track of requested IDs
    mutable unsigned int numIdRequested_;

    // High-level method to create a new cell. This allocates a new CellData
    // and inserts it in cellsAfter_.
    //
    template <class CellDataType> // e.g., CellDataType = KeyVertexData<Ids>
    CellDataType * newCell_(CellId * outId)
    {
        // Get available ID
        CellId id = getAvailableId_();
        if (outId)
            *outId = id;

        // Allocate cell data
        CellDataType * cellData = new CellDataType();
        UniquePtr<CellDataType> cellDataUniquePtr(cellData);

        // Attempt to insert cellData in cellsAfter_. Ownership of cellData is
        // transferred to cellAfter_.
        auto inserted = cellsAfter_.insert(
                 std::make_pair(id, std::move(cellDataUniquePtr)));

        // Abort if insertion failed (i.e., ID already taken)
        assert(inserted.second);

        // Tag this ID as a cell created by the operator
        newCells_.push_back(id);

        // Return the allocated OpCellData
        return cellData;
    }

    // High-level method to get an existing cell. It first determines whether
    // a CellData with the given ID already exists in cellsAfter_, in which
    // case it is returned. Otherwise, it finds the Cell with the given ID in
    // the Vac, and creates a copy of its data both in cellsBefore_ and
    // cellsAfter_. The copy in cellsAfter_ is static casted to the requested
    // type and returned.
    //
    template <class CellDataType> // e.g., CellDataType = KeyVertexData<Ids>
    CellDataType * getCell_(CellId id)
    {
        // Is the data already in cellsAfter_?
        auto it = cellsAfter_.find(id);
        if (it != cellsAfter_.end())
        {
            // If yes, just retrieve it
            CellData<Ids> * cellDataAfter = it->second.get();

            // Check that it has the same type
            if (cellDataAfter->type() != CellDataType::static_type())
            {
                // That's a bug in compute_(): this cell has already
                // been affected by compute_(), but now it's trying
                // to get it again with a different type.
                abort();
            }

            // Cast to correct type and return
            return static_cast<CellDataType *>(cellDataAfter);
        }

        // If not, then it musn't be in cellsBefore_ (that would
        // mean the compute_() function called deleteCell(), which
        // is the only way to have a cell ID in cellBefore_ but not
        // in cellAfter_. That's a bug in compute_() and therefore
        // aborts.
        assert(cellsBefore_.find(id) == cellsBefore_.end());

        // And it must be in the VAC, otherwise the ID exists nowhere!
        // That's not necessarilly a bug though, compute_() might just
        // be querying if the cell exists to know whether the operation
        // is legal or not.
        auto it2 = vac_->cellManager_.find(id);
        if (it2 == vac_->cellManager_.end())
        {
            return nullptr;
        }

        // Get Vac cell
        const SharedPtr<Cell> & vacCell = it2->second;

        // Check type
        if (vacCell->type() != CellDataType::static_type())
        {
            // That's not necessarilly a bug. The compute_() might
            // be testing whether the cell exists and has the correct
            // type
            return nullptr;
        }

        // Get Vac cell data
        const CellData<Handles> & vacCellData = vacCell->data();

        // Allocate Operator cell data before and after
        CellDataType * opCellDataBefore = new CellDataType();
        CellDataType * opCellDataAfter = new CellDataType();
        UniquePtr<CellData<Ids>> opCellDataBeforeUniquePtr(opCellDataBefore);
        UniquePtr<CellData<Ids>> opCellDataAfterUniquePtr(opCellDataAfter);

        // Attempt to insert in cellsBefore_ and cellsAfter_. Ownership
        // is transfered to these containers.
        auto insertedBefore = cellsBefore_.insert(
                std::make_pair(id, std::move(opCellDataBeforeUniquePtr)));
        auto insertedAfter = cellsAfter_.insert(
                std::make_pair(id, std::move(opCellDataAfterUniquePtr)));

        // Abort if any insertion failed (i.e., ID already taken)
        assert(insertedBefore.second);
        assert(insertedAfter.second);

        // Copy cell data from Vac to Operator
        handlesToIdsCopier_.copy(vacCellData, *opCellDataBefore);
        handlesToIdsCopier_.copy(vacCellData, *opCellDataAfter);

        // Returns the relevant op cell data
        return opCellDataAfter;
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

    SharedPtr<Cell> makeCell_(Vac * vac, CellType type, CellId id) const
    {
        #define OPENVAC_OPERATOR_RETURN_MAKE_SHARED_IF_TYPE_MATCHES_(CellType_) \
            if (type == CellType::CellType_) \
                return std::make_shared<CellType_>(vac, id);

        // Switch over cell types
        OPENVAC_FOREACH_FINAL_CELL_TYPE(
                    OPENVAC_OPERATOR_RETURN_MAKE_SHARED_IF_TYPE_MATCHES_)

        // Abort if incorrect cell type
        abort();
    }
};

/// \namespace OpenVac::Operators
/// \brief Free functions to perform an in-place mofification of a Vac.
///
namespace Operators
{
}

}

#endif // OPENVAC_OPERATOR_H
