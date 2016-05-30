// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef VGEOMETRY_QUADRATICCURVE_H
#define VGEOMETRY_QUADRATICCURVE_H

#include <glm/vec2.hpp>
#include <vector>

namespace VGeometry
{

/// \class QuadraticCurve
/// \brief A class that represents a 2D curve as a quadratic polynomial of glm::dvec2.
///
/// This class represents a curve as the quadratic polynomial:
///
///     a + b*u + c*u^2
///
/// Where each of a, b, and c are a glm::dvec2. Note that this is not a
/// spline, it is just a unique piece of quadratic polynomial. Also, it does not
/// provide arclength parameterization.
///
/// You can evaluate the curve using pos(u), evaluates is derivative using
/// der(u), and evaluate its second derivative using der2(u).
///
class QuadraticCurve
{
public:
    /// Constructs an uninitialized QuadraticCurve.
    ///
    QuadraticCurve() {}

    /// Constructs a QuadraticCurve with the given polynomial coefficients.
    ///
    QuadraticCurve(const glm::dvec2 & a,
                   const glm::dvec2 & b,
                   const glm::dvec2 & c) :
        a_(a),
        b_(b),
        c_(c)
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

    /// Returns a QuadraticCurve whose polynomial coefficients are converted from
    /// the four control points of a cubic Bézier.
    ///
    inline static QuadraticCurve fromBezier(const glm::dvec2 & p0,
                                            const glm::dvec2 & p1,
                                            const glm::dvec2 & p2)
    {
        return QuadraticCurve(
                    p0,
                    2.0 * (p1 - p0),
                    p2 - 2.0 * p1 + p0
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

    /// Returns the position of the QuadraticCurve at \p u in [0,1].
    ///
    inline glm::dvec2 pos(double u) const
    {
        return a_ + u*(b_ + u*c_); // a + u*b + u*u*c
    }

    /// Returns the derivative of the QuadraticCurve at \p u in [0,1].
    ///
    inline glm::dvec2 der(double u) const
    {
        return b_ + (u+u)*c_; // b + 2*u*c
    }

    /// Returns the second derivative of the QuadraticCurve at \p u in [0,1].
    ///
    inline glm::dvec2 der2(double /*u*/) const
    {
        return c_+c_; // 2*c
    }

private:
    // Polynomial coefficients
    glm::dvec2 a_, b_, c_;
};

} // end namespace VGeometry

#endif // VGEOMETRY_QUADRATICCURVE_H
