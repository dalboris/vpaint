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

template <class VacPtr>
struct get_geometry_type
{
    typedef typename VacPtr::element_type::geometry_type type;

};

template <class Vac>
struct get_geometry_type<Vac*>
{
    typedef typename Vac::geometry_type type;

};

template <class VacPtr>
using geometry_type = typename get_geometry_type<VacPtr>::type;

}

#endif
