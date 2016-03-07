// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_OPMAKEKEYVERTEX_H
#define OPENVAC_OPMAKEKEYVERTEX_H

#include <OpenVac/Operators/Operator.h>

namespace OpenVac
{

/// \addtogroup Operators
/// @{

template <class Geometry>
class OpMakeKeyVertex: public Operator<Geometry>
{
public:
    OPENVAC_OPERATOR(OpMakeKeyVertex)

    // Constructor
    OpMakeKeyVertex(VacPtr vac, typename Geometry::Frame frame) :
        Operator(vac),
        frame_(frame) {}

    // Post-computation info. Aborts if not computed.
    KeyVertexId keyVertexId() const { assert(isComputed()); return keyVertexId_; }

    // Post-application info. Aborts if not applied.
    KeyVertexHandle keyVertex() const { assert(isApplied()); return vac()->cell(keyVertexId()); }

private:
    typename Geometry::Frame frame_;
    bool isValid_() { return true; }

    KeyVertexId keyVertexId_;
    void compute_()
    {
        auto keyVertex = newKeyVertex(&keyVertexId_);
        keyVertex->frame = frame_;
    }
};

namespace Operators
{

/// \fn template <class VacPtr> OpMakeKeyVertex<geometry_type<VacPtr>> MakeKeyVertex(VacPtr & vac, Frame frame)
///
/// Constructs an OpMakeKeyVertex<Geometry> without having to specify the
/// template parameter (thanks to template argument deduction).
///
/// \code
/// auto op = Operators::MakeKeyVertex(vac, frame);
/// \endcode
///
template <class VacPtr>
OpMakeKeyVertex<geometry_type_t<VacPtr>> MakeKeyVertex(
        VacPtr & vac,
        typename geometry_type_t<VacPtr>::Frame frame)
{
    return OpMakeKeyVertex<geometry_type_t<VacPtr>>(vac, frame);
}

/// Constructs an OpMakeKeyVertex<Geometry>, applies the operation, then returns
/// the created vertex.
///
/// \code
/// auto keyVertex = Operators::makeKeyVertex(vac, frame);
/// \endcode
///
template <class VacPtr>
KeyVertexHandle<geometry_type_t<VacPtr>> makeKeyVertex(
        VacPtr & vac,
        typename geometry_type_t<VacPtr>::Frame frame)
{
    return OpMakeKeyVertex<geometry_type_t<VacPtr>>(vac, frame).apply().keyVertex();
}

}

}

/// @} Doxygen Operators group

#endif
