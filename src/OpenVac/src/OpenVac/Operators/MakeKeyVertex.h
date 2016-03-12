// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_MAKEKEYVERTEX_H
#define OPENVAC_MAKEKEYVERTEX_H

#include <OpenVac/Operators/Operator.h>

namespace OpenVac
{

/// \addtogroup Operators
/// @{

/// \class OpMakeKeyVertex OpenVac/Operators/MakeKeyVertex.h
/// \brief Operator to create a new key vertex
///
template <class Geometry>
class OpMakeKeyVertex: public Operator<Geometry>
{
    OPENVAC_OPERATOR(OpMakeKeyVertex)

public:
    /// Constructs an OpMakeKeyVertex.
    OpMakeKeyVertex(Frame frame) :
        frame_(frame)
    {
    }

    /// Returns the ID of the new key vertex. Aborts if can't be applied.
    ///
    KeyVertexId keyVertexId() const
    {
        assert(canBeApplied());
        return keyVertexId_;
    }

private:
    // Input
    Frame frame_;

    // Output
    KeyVertexId keyVertexId_;

    // Implementation
    bool compute_()
    {
        auto keyVertex = newKeyVertex(&keyVertexId_);
        keyVertex->frame = frame_;

        return true;
    }
};

namespace Operators
{

/// Constructs an OpMakeKeyVertex.
///
/// \code
/// auto op = Operators::MakeKeyVertex(frame);
/// if (op.compute(vac))
///     op.apply(vac);
/// \endcode
///
template <class Geometry>
OpMakeKeyVertex<Geometry> MakeKeyVertex(typename Geometry::Frame frame)
{
    return OpMakeKeyVertex<Geometry>(frame);
}

/// Constructs an OpMakeKeyVertex<Geometry>, applies the operation to the given
/// \p vac, then returns the created vertex. Returns an empty handle if the
/// operator can't be applied.
///
/// \code
/// auto keyVertex = Operators::makeKeyVertex(vac, frame);
/// \endcode
///
template <class Geometry>
Handle<KeyVertex<Geometry>> makeKeyVertex(Vac<Geometry> & vac,
                                          typename Geometry::Frame frame)
{
    Handle<KeyVertex<Geometry>> res;
    auto op = MakeKeyVertex<Geometry>(frame);
    if (op.compute(vac))
    {
        op.apply(vac);
        res = vac.cell(op.keyVertexId());
    }
    return res;
}

} // end namespace Operators

} // end namespace OpenVac

/// @} end addtogroup Operators

#endif // OPENVAC_MAKEKEYVERTEX_H
