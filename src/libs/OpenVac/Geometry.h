// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

/// \file OpenVac/Geometry.h
///
/// In this file are declared all geometric types to be used by OpenVac.
/// This file is meant to be customized by users of OpenVac.

#ifndef OPENVAC_GEOMETRY_H
#define OPENVAC_GEOMETRY_H

#include "VGeometry.h"

namespace OpenVac
{

/// \namespace OpenVac::Geometry
/// \brief A namespace to specify which geometric types OpenVac should use.
///
/// This namespace declares all geometric types to be used by OpenVac. It is
/// meant to be customized by users of OpenVac. Think of this namespace
/// as a template parameter that is automatically passed to all OpenVac
/// classes.
///
/// Well, then, you might wonder why isn't OpenVac actually implemented with
/// templates, instead of this weird namespace? Why not let the user define a
/// custom trait class 'MyGeometry', then use OpenVac::Vac<MyGeometry>?
///
/// This is a very legitimate question, and indeed templates intuitively seem
/// to be the appropriate C++ concept for the job. Unfortunately, we did try
/// that, and it turned out not to be worth the trouble. With templates,
/// OpenVac was much harder to develop, its code was much less readable, and
/// eventually that would translate into more bugs and less features. Also, and
/// perhaps more importantly, it was making client code harder to develop too.
/// Learning how to use OpenVac was harder, compile error messages were
/// cryptic, using a debugger was tricky, and client code itself was less
/// readable, even after aliasing everything that could be aliased (template
/// argument deduction sometimes fails). Also, compilation times were longer,
/// and IDE (e.g. QtCreator) were sometimes getting slow -or even failing- to
/// provide meaningful automcompletion.
///
/// So at the end, we decided not to use templates, and instead to use this
/// 'Geometry' namespace. In a sense, templates are an overkill for OpenVac
/// since most likely, you would only use OpenVac<Geometry> with a unique
/// Geometry template parameter, unlike std::vector<T>, which is typically used
/// with various T within the same application. Though, this means that to
/// customize Geometry, you need to write code directly *inside* the library,
/// which has unfortunately a few drawbacks:
///
///   - OpenVac can't be used as a system-wide shared library: it needs to be
///     shipped with your application.
///
///   - If you need to have two Vac classes using different Geometry within the
///     same application, then you need to actually duplicate the OpenVac
///     library, and put them in different namespaces (i.e., rename the two
///     'OpenVac' namespace into 'MyOpenVac1' and 'MyOpenVac2'), effectively
///     having two separate libraries with almost identical code. It's quite
///     ugly, but most applications don't need that functionality, and even for
///     those who do, we still think it is a better evil than using templates.
///     Note that duplication and renaming can be automated in the build
///     process by a script, conceptually doing the same as what C++ compilers
///     do with templates, but as a pre-compilation step.
///
///   - When updating to a new version of OpenVac, you should replace the
///     new OpenVac/Geometry.h by your own version of Geometry.h.
///
/// The following types must be declared:
///
///   - Frame
///   - KeyVertexGeometry
///   - KeyEdgeGeometry
///   - GeometryManager
///
namespace Geometry
{
}

} // end namespace OpenVac

#endif // OPENVAC_GEOMETRY_H
