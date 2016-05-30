// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef VGEOMETRY_CUBICCURVE_H
#define VGEOMETRY_CUBICCURVE_H

#include "QuadraticCurve.h"

#include <glm/vec2.hpp>
#include <vector>

namespace VGeometry
{

/// \class CubicCurve
/// \brief A class that represents a 2D curve as a cubic polynomial of glm::dvec2.
///
/// This class represents a curve as the cubic polynomial:
///
///     a + b*u + c*u^2 + d*u^3
///
/// Where each of a, b, c, and d is a glm::dvec2. Note that this is not a
/// spline, it is just a unique piece of cubic polynomial. Also, it does not
/// provide arclength parameterization.
///
/// You can evaluate the curve using pos(u), evaluates is derivative using
/// der(u), and evaluate its second derivative using der2(u).
///
class CubicCurve
{
public:
    /// Constructs an uninitialized CubicCurve.
    ///
    CubicCurve() {}

    /// Constructs a CubicCurve with the given polynomial coefficients.
    ///
    CubicCurve(const glm::dvec2 & a,
               const glm::dvec2 & b,
               const glm::dvec2 & c,
               const glm::dvec2 & d) :
        a_(a),
        b_(b),
        c_(c),
        d_(d)
    {}

    /// Constructs a CubicCurve from a QuadraticCurve. This copies the a, b,
    /// and c polynomial coefficients, and sets d to zero.
    ///
    CubicCurve(const QuadraticCurve & q) :
        a_(q.a()),
        b_(q.b()),
        c_(q.c()),
        d_(0.0, 0.0)
    {}

    /// Sets the "a" polynomial coefficient.
    ///
    inline void setA(const glm::dvec2 & a)
    {
        a_ = a;
    }

    /// Sets the "b" polynomial coefficient.
    ///
    inline void setB(const glm::dvec2 & b)
    {
        b_ = b;
    }

    /// Sets the "c" polynomial coefficient.
    ///
    inline void setC(const glm::dvec2 & c)
    {
        c_ = c;
    }

    /// Sets the "d" polynomial coefficient.
    ///
    inline void setD(const glm::dvec2 & d)
    {
        d_ = d;
    }

    /// Returns a CubicCurve whose polynomial coefficients are converted from
    /// the four control points of a cubic Bézier.
    ///
    inline static CubicCurve fromBezier(const glm::dvec2 & p0,
                                        const glm::dvec2 & p1,
                                        const glm::dvec2 & p2,
                                        const glm::dvec2 & p3)
    {
        return CubicCurve(
                    p0,
                    3.0 * (p1 - p0),
                    3.0 * (p2 - 2.0 * p1 + p0),
                    p3 + 3.0 * (p1 - p2) - p0
               );
    }

    /// Returns the "b" polynomial coefficient.
    ///
    inline glm::dvec2 a() const
    {
        return a_;
    }

    /// Returns the "b" polynomial coefficient.
    ///
    inline glm::dvec2 b() const
    {
        return b_;
    }

    /// Returns the "c" polynomial coefficient.
    ///
    inline glm::dvec2 c() const
    {
        return c_;
    }

    /// Returns the "d" polynomial coefficient.
    ///
    inline glm::dvec2 d() const
    {
        return d_;
    }

    /// Returns the position of the CubicCurve at \p u in [0,1].
    ///
    inline glm::dvec2 pos(double u) const
    {
        return a_ + u*(b_ + u*(c_ + u*d_)); // a_ + u*b_ + u*u*c_ + u*u*u*d_
    }

    /// Returns the derivative of the CubicCurve at \p u in [0,1].
    ///
    inline glm::dvec2 der(double u) const
    {
        return b_ + u*((c_+c_) + 3*u*d_); // b + 2*u*c + 3*u*u*d
    }

    /// Returns the second derivative of the CubicCurve at \p u in [0,1].
    ///
    inline glm::dvec2 der2(double u) const
    {
        return (c_+c_) + 6*u*d_; // 2*c + 6*u*d
    }

private:
    // Polynomial coefficients
    glm::dvec2 a_, b_, c_, d_;
};

} // end namespace VGeometry

#endif // VGEOMETRY_CUBICCURVE_H
