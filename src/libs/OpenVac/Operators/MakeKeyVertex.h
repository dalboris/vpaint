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

/// \class OpMakeKeyVertex OpenVac/Operators/MakeKeyVertex.h
/// \brief Operator to create a key vertex.
///
class OpMakeKeyVertex: public Operator
{
public:
    /// Overrides compute to return derived type.
    OpMakeKeyVertex & compute(const Vac & vac) { Operator::compute(vac); return *this; }

    /// Overrides apply to return derived type.
    OpMakeKeyVertex & apply(Vac & vac)         { Operator::apply(vac);   return *this; }

    /// Constructs an OpMakeKeyVertex.
    OpMakeKeyVertex(
            const Geometry::Frame & frame = Geometry::Frame(),
            const Geometry::KeyVertexGeometry & geometry = Geometry::KeyVertexGeometry()) :
        frame_(frame),
        geometry_(geometry)
    {
    }

    /// Returns the ID of the created key vertex. Aborts if can't be applied.
    KeyVertexId keyVertexId() const
    {
        assert(canBeApplied());
        return keyVertexId_;
    }

private:
    // Input
    Geometry::Frame frame_;
    Geometry::KeyVertexGeometry geometry_;

    // Output
    KeyVertexId keyVertexId_;

    // Implementation
    bool compute_()
    {
        auto keyVertex = newKeyVertex(&keyVertexId_);
        keyVertex->frame = frame_;
        keyVertex->geometry = geometry_;

        return true;
    }
};

namespace Operators
{

/// Creates a new KeyVertex in the given Vac, at the given Frame, with the
/// given KeyVertexGeometry.
///
/// \code
/// KeyVertexHandle keyVertex = Operators::makeKeyVertex(vac, frame, geometry);
/// \endcode
///
inline KeyVertexHandle makeKeyVertex(
        Vac & vac,
        const Geometry::Frame & frame = Geometry::Frame(),
        const Geometry::KeyVertexGeometry & geometry = Geometry::KeyVertexGeometry())
{
    KeyVertexHandle res;
    OpMakeKeyVertex op(frame, geometry);
    if (op.compute(vac))
    {
        op.apply(vac);
        res = vac.cell(op.keyVertexId());
    }
    return res;
}

} // end namespace Operators

} // end namespace OpenVac

#endif // OPENVAC_MAKEKEYVERTEX_H
