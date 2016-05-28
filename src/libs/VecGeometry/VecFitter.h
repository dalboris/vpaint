// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef VECFITTER_H
#define VECFITTER_H

#include <glm/vec2.hpp>
#include <vector>

class VecFitter
{
public:
    /// Constructs an uninitialized VecFitter.
    ///
    VecFitter() {}

    /// Constructs a VecFitter that fits the given \p samples.
    ///
    VecFitter(const std::vector<glm::vec2> & samples);

    /// Returns the sample after fitting, given \p u in [0,1].
    ///
    inline glm::dvec2 operator()(double u) const
    {
        const double u2 = u*u;
        const double u3 = u2*u;
        return a_ + u*b_ + u2*c_ + u3*d_;
    }

    /// Returns the derivative after fitting, given \p u in [0,1].
    ///
    inline glm::dvec2 der(double u) const
    {
        return b_ + 2*u*c_ + 3*u*u*d_;
    }

    /// Returns the second derivative after fitting, given \p u in [0,1].
    ///
    inline glm::dvec2 der2(double u) const
    {
        return 2.0*c_ + 6*u*d_;
    }

    /// Returns the parameter corresponding to the fitted samples
    ///
    inline const std::vector<double> & uis() const { return uis_; }

private:
    // Cubic parameters
    glm::dvec2 a_, b_, c_, d_;

    // sample parameters
    std::vector<double> uis_;

    // Set canonical parameter from bezier parameters
    void setFromQuadraticBezier_(const glm::dvec2 & p0, const glm::dvec2 & p1, const glm::dvec2 & p2);
    void setFromCubicBezier_(const glm::dvec2 & p0, const glm::dvec2 & p1, const glm::dvec2 & p2, const glm::dvec2 & p3);
};

#endif // VECFITTER_H
