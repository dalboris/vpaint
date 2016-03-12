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

namespace OpenVac
{

/*************************** Operator subclass *******************************/

/// \class OpMakeKeyOpenEdge OpenVac/Operators/OpMakeKeyOpenEdge.h
/// \brief Operator to create a key open edge
///
template <class Geometry>
class OpMakeKeyOpenEdge: public Operator<Geometry>
{
    OPENVAC_OPERATOR(OpMakeKeyOpenEdge)

public:
    /// Constructs an OpMakeKeyOpenEdge.
    OpMakeKeyOpenEdge(KeyVertexId startVertexId, KeyVertexId endVertexId) :
        startVertexId_(startVertexId),
        endVertexId_(endVertexId)
    {
    }

    /// Returns ID of new edge. Aborts if can't be applied.
    KeyEdgeId keyEdgeId() const
    {
        assert(canBeApplied());
        return keyEdgeId_;
    }

private:
    // Input
    KeyVertexId startVertexId_, endVertexId_;

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

/// Constructs an OpMakeKeyOpenEdge<Geometry>.
///
/// \code
/// auto op = Operators::MakeKeyOpenEdge(startvertex, endVertex);
/// \endcode
///
template <class Geometry>
OpMakeKeyOpenEdge<Geometry> MakeKeyOpenEdge(
        const Handle<KeyVertex<Geometry>> & startVertex,
        const Handle<KeyVertex<Geometry>> & endVertex)
{
    return OpMakeKeyOpenEdge<Geometry>(startVertex->id(), endVertex->id());
}

/// Constructs an OpMakeKeyOpenEdge<Geometry>, applies the operation, then
/// returns the created edge.
///
/// \code
/// auto keyEdge = Operators::makeKeyOpenEdge(startVertex, endVertex);
/// \endcode
///
template <class Geometry>
Handle<KeyEdge<Geometry>> makeKeyOpenEdge(
        const Handle<KeyVertex<Geometry>> & startVertex,
        const Handle<KeyVertex<Geometry>> & endVertex)
{
    Handle<KeyEdge<Geometry>> res;
    Vac<Geometry> & vac = *startVertex->vac();
    auto op = MakeKeyOpenEdge<Geometry>(startVertex, endVertex);
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
