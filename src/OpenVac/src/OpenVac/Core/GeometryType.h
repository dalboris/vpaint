// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef OPENVAC_GEOMETRYTYPE_H
#define OPENVAC_GEOMETRYTYPE_H

namespace OpenVac
{
///
/// \struct geometry_type
///
/// Provides a member typedef \p type to the template parameter \p Geometry of
/// the type \p T<Geometry> pointed to by \p T_Ptr
///
/// The following pointer types T_Ptr are supported:
///   1. Raw pointers
///   2. std::shared_ptr
///   3. std::weak_ptr
///   4. OpenVac::SharedPtr
///   5. OpenVac::WeakPtr
///
/// The following types T are supported:
///   1. Vac
///
/// <H2>Helper type:</H2>
///
/// \code
/// template< class T_Ptr >
/// using geometry_type_t = typename geometry_type<T_Ptr>::type;
/// \endcode
///
/// <H2>Example:</H2>
///
/// \code
/// template <class VacPtr>
/// OpMakeKeyVertex<geometry_type_t<VacPtr>> MakeKeyVertex(VacPtr & vac, Frame frame)
/// {
///     return OpMakeKeyVertex<geometry_type_t<VacPtr>>(vac, frame);
/// }
/// \endcode

// Generic version for smart pointers
template <class VacPtr>
struct geometry_type
{
    typedef typename VacPtr::element_type::geometry_type type;

};

/// \cond
///
// Specialization for raw pointers
template <class Vac>
struct geometry_type<Vac*>
{
    typedef typename Vac::geometry_type type;

};

// Helper type
template <class T_Ptr>
using geometry_type_t = typename geometry_type<T_Ptr>::type;
///
/// \endcond

}

#endif
