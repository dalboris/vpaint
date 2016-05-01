// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_MAKEKEYEDGE_H
#define OPENVAC_MAKEKEYEDGE_H

#include <OpenVac/Operators/Operator.h>
#include <OpenVac/Topology/KeyVertex.h>

namespace OpenVac
{

/// \class OpMakeKeyOpenEdge OpenVac/Operators/MakeKeyOpenEdge.h
/// \brief Operator to create a key open edge
///
class OpMakeKeyOpenEdge: public Operator
{
public:
    /// Overrides compute to return derived type.
    OpMakeKeyOpenEdge & compute(const Vac & vac) { Operator::compute(vac); return *this; }

    /// Overrides apply to return derived type.
    OpMakeKeyOpenEdge & apply(Vac & vac)         { Operator::apply(vac);   return *this; }

    /// Constructs an OpMakeKeyOpenEdge.
    OpMakeKeyOpenEdge(
            KeyVertexId startVertexId,
            KeyVertexId endVertexId,
            const Geometry::KeyEdgeGeometry & geometry = Geometry::KeyEdgeGeometry()) :
        startVertexId_(startVertexId),
        endVertexId_(endVertexId),
        geometry_(geometry)
    {
    }

    /// Returns the ID of created key edge. Aborts if can't be applied.
    KeyEdgeId keyEdgeId() const
    {
        assert(canBeApplied());
        return keyEdgeId_;
    }

private:
    // Input
    KeyVertexId startVertexId_, endVertexId_;
    Geometry::KeyEdgeGeometry geometry_;

    // Output
    KeyEdgeId keyEdgeId_;

    // Implementation
    bool compute_()
    {
        auto startVertex = getKeyVertex(startVertexId_);
        auto endVertex   = getKeyVertex(endVertexId_);

        if (startVertex &&
            endVertex &&
            startVertex->frame == endVertex->frame)
        {
            auto keyEdge = newKeyEdge(&keyEdgeId_);
            keyEdge->startVertex = startVertexId_;
            keyEdge->endVertex   = endVertexId_;
            keyEdge->frame       = startVertex->frame;

            return true;
        }
        else
        {
            return false;
        }
    }
};

/**************************** Convenient API *********************************/

namespace Operators
{

/// Creates a new KeyEdge in the given Vac, starting at the given startVertex
/// and ending at the given endVertex, with the given KeyEdgeGeometry. Returns
/// a handle to the new KeyEdge, or an empty handle if the key edge cannot be
/// created (for instance, if its end vertices are not on the same frame).
/// Behavior is undefined if one of the given key vertex handles are empty.
///
/// \code
/// KeyEdgeHandle keyEdge = Operators::makeKeyOpenEdge(startVertex, endVertex);
/// \endcode
///
KeyEdgeHandle makeKeyOpenEdge(
        const KeyVertexHandle & startVertex,
        const KeyVertexHandle & endVertex,
        const Geometry::KeyEdgeGeometry & geometry = Geometry::KeyEdgeGeometry())
{
    KeyEdgeHandle res;
    Vac & vac = *startVertex->vac();
    OpMakeKeyOpenEdge op(startVertex->id(), endVertex->id(), geometry);
    if (op.compute(vac))
    {
        op.apply(vac);
        res = vac.cell(op.keyEdgeId());
    }
    return res;
}

} // end namespace Operators

} // end namespace OpenVac

#endif // OPENVAC_OPMAKEKEYEDGE_H
