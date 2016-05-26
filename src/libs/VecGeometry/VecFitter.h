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
    inline glm::vec2 operator()(float u) const
    {
        const float u2 = u*u;
        const float u3 = u2*u;
        return a_ + u*b_ + u2*c_ + u3*d_;
    }

private:
    // Cubic parameters
    glm::vec2 a_, b_, c_, d_;
};

#endif // VECFITTER_H
