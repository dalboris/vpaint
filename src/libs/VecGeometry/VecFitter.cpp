// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "VecFitter.h"

#include <Eigen/Core>
#include <Eigen/LU>
#include <Eigen/StdVector>

VecFitter::VecFitter(const std::vector<glm::vec2> & samples)
{
    const size_t n = samples.size();
    const glm::vec2 zero(0.0f, 0.0f);

    if (n == 0) // constant to zero
    {
        a_ = zero;
        b_ = zero;
        c_ = zero;
        d_ = zero;
    }
    else if (n == 1) // constant to samples[0]
    {
        a_ = samples[0];
        b_ = zero;
        c_ = zero;
        d_ = zero;
    }
    else if (n == 2) // segment from samples[0] to samples[1]
    {
        a_ = samples[0];
        b_ = samples[1] - samples[0];
        c_ = zero;
        d_ = zero;
    }
    else if (n == 3) // Quadratic bezier, with the 3 control points
                     // samples[0], samples[1], and samples[2]
    {
        a_ = samples[0];
        b_ = 2.0f * (samples[1] - samples[0]);
        c_ = samples[2] - 2.0f * samples[1] + samples[0];
        d_ = zero;
    }
    else if (n == 4) // Cubic bezier, with the 4 control points
                     // samples[0], samples[1], samples[2], and samples[3]
    {

        a_ = samples[0];
        b_ = 3.0f * (samples[1] - samples[0]);
        c_ = 3.0f * (samples[2] - 2.0f * samples[1] + samples[0]);
        d_ = samples[3] + 3.0f * (samples[1] - samples[2]) - samples[0];
    }
    else // Cubic bezier fitting the input
    {
        // First and last bezier point (p0 and p3)
        const glm::vec2 & p0 = samples[0];
        const glm::vec2 & p3 = samples[n-1];

        // Build linear system to solve for the two other bezier points (p1 and p2)
        Eigen::MatrixXd A(2*(n-2),4);
        Eigen::VectorXd B(2*(n-2));
        const double invNMinusOne = 1.0 / (double) (n-1);
        for(unsigned int i=1; i<n-1; i++)
        {
            const double Ui  = invNMinusOne * (double) i;
            const double Ui2 = Ui*Ui;
            const double Ui3 = Ui2*Ui;

            const double OneMinusUi  = 1 - Ui;
            const double OneMinusUi2 = OneMinusUi*OneMinusUi;
            const double OneMinusUi3 = OneMinusUi2*OneMinusUi;

            const double ThreeOneMinusUi2Ui = 3 * OneMinusUi2 * Ui;
            const double ThreeOneMinusUiUi2 = 3 * OneMinusUi * Ui2;

            const unsigned int ix = 2*i-2;
            const unsigned int iy = 2*i-1;

            A(ix, 0) = ThreeOneMinusUi2Ui;
            A(ix, 1) = 0;
            A(ix, 2) = ThreeOneMinusUiUi2;
            A(ix, 3) = 0;

            A(iy, 0) = 0;
            A(iy, 1) = ThreeOneMinusUi2Ui;
            A(iy, 2) = 0;
            A(iy, 3) = ThreeOneMinusUiUi2;

            B(ix) = samples[i].x - OneMinusUi3 * p0.x - Ui3 * p3.x;
            B(iy) = samples[i].y - OneMinusUi3 * p0.y - Ui3 * p3.y;
        }

        // Solve linear system
        Eigen::MatrixXd M = (A.transpose() * A).inverse();
        M *= A.transpose();
        Eigen::VectorXd X = M * B;
        const glm::vec2 p1(X(0), X(1));
        const glm::vec2 p2(X(2), X(3));

        // Create cubic polynomial coefficient from bezier points
        a_ = p0;
        b_ = 3.0f * (p1 - p0);
        c_ = 3.0f * (p2 - 2.0f * p1 + p0);
        d_ = p3 + 3.0f * (p1 - p2) - p0;
    }
}
