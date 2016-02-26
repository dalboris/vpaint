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

#include <vector>
#include <map>

namespace OpenVAC
{

class VAC;

class Operator
{
public:
    // Constructor
    Operator(VAC * vac = nullptr);

    // Returns the VAC this operator is bound to.
    VAC * vac() const;

    // Checks whether the operation is valid.
    bool isValid();

    // Computes operation. Does nothing if already computed.
    // Aborts if not valid. Returns this Operator.
    Operator & compute();
    bool isComputed() const;

    // Computes operation if not computed yet, then applies operation to VAC.
    // Aborts if not valid or already applied. Returns this Operator.
    Operator & apply();
    bool isApplied() const;

    // Get info about operation (compute and/or apply must have been called)
    const std::vector<CellId> & newCells();
    const std::vector<CellId> & deletedCells();

protected:
    // Methods that must be implemented by derived classes
    virtual bool isValid_()=0;
    virtual void compute_()=0;

    // Methods to be used by derived classes
    OpKeyVertexDataPtr newKeyVertex(KeyVertexId * outId = nullptr);
    OpKeyEdgeDataPtr   newKeyEdge  (KeyEdgeId *   outId = nullptr);

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
    void apply_();
    bool isApplied_;

    // Keep track of requested IDs
    mutable unsigned int numIdRequested_;

    // Private methods
    CellId getAvailableId_() const;
    std::vector<CellId> getAvailableIds_(unsigned int numIds) const;
    CellSharedPtr make_shared(CellType type, CellId id) const;
    template <class OpCellDataType>
    friend WeakPtr<OpCellDataType> Operator_newCell_(Operator & self, CellId * outId);
};

// Convenient macros for derived operators. It defines an override of
// Operator::compute() and Operator::apply() to return a reference to an
// OpDerivedType instead of a reference to an Operator
#define OPENVAC_OPERATOR_OVERRIDE_COMPUTE_AND_APPLY(OpType) \
    OpType & compute() { Operator::compute(); return *this;} \
    OpType & apply()   { Operator::apply();   return *this;}

}

#endif // OPENVAC_OPERATOR_H
