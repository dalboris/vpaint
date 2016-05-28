// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "VecFitter.h"

#include <glm/geometric.hpp>
#include <Eigen/Core>
#include <Eigen/LU>
#include <Eigen/StdVector>

#include <glm/vec2.hpp>

#include <iostream>

namespace
{

// Compute p1 and p2 such that (p0, p1, p2, p3) is the
// cubic bezier B that minimizes \sum (samples[i] - B(u[i]))^2
//
// samples.size() must be > 3
//
void solveCubicLeastSquare_(
        const std::vector<glm::vec2> & samples,
        const std::vector<double> & u,
        const glm::dvec2 & p0,
        glm::dvec2 & p1,
        glm::dvec2 & p2,
        const glm::dvec2 & p3)
{
    const size_t n = samples.size();
    assert(n>3);

    // Build matrices A and B such that the least square problem
    // is expressed as:
    //
    //     min || AX - B ||^2
    //
    Eigen::MatrixXd A(2*(n-2),4);
    Eigen::VectorXd B(2*(n-2));
    for(unsigned int i=1; i<n-1; i++)
    {
        const double Ui  = u[i];
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

    // Solving
    //     min || AX - B ||^2
    // Is equivalent to solving
    //     At A X - At B = 0
    // Therefore:
    //     X = (At A)^(-1) At B
    Eigen::MatrixXd M = (A.transpose() * A).inverse();
    M *= A.transpose();
    Eigen::VectorXd X = M * B;
    p1.x = X(0);
    p1.y = X(1);
    p2.x = X(2);
    p2.y = X(3);
}

// Compute p1 such that (p0, p1, p2) is the
// quadratic bezier B that minimizes \sum (samples[i] - B(u[i]))^2
//
// samples.size() must be > 2
//
void solveQuadraticLeastSquare_(
        const std::vector<glm::vec2> & samples,
        const std::vector<double> & u,
        const glm::dvec2 & p0,
        glm::dvec2 & p1,
        const glm::dvec2 & p2)
{
    const size_t n = samples.size();
    assert(n>2);

    // Build matrices A and B such that the least square problem
    // is expressed as:
    //
    //     min || AX - B ||^2
    //
    Eigen::MatrixXd A(2*(n-2),2);
    Eigen::VectorXd B(2*(n-2));
    for(unsigned int i=1; i<n-1; i++)
    {
        const double Ui  = u[i];
        const double Ui2 = Ui*Ui;
        const double TwoOneMinusUiUi = 2 * (1-Ui) * Ui;
        const double OneMinusUi2 = (1-Ui) * (1-Ui);

        const unsigned int ix = 2*i-2;
        const unsigned int iy = 2*i-1;

        A(ix, 0) = TwoOneMinusUiUi;
        A(ix, 1) = 0;

        A(iy, 0) = 0;
        A(iy, 1) = TwoOneMinusUiUi;

        B(ix) = samples[i].x - OneMinusUi2 * p0.x - Ui2 * p2.x;
        B(iy) = samples[i].y - OneMinusUi2 * p0.y - Ui2 * p2.y;
    }

    // Solving
    //     min || AX - B ||^2
    // Is equivalent to solving
    //     At A X - At B = 0
    // Therefore:
    //     X = (At A)^(-1) At B
    Eigen::MatrixXd M = (A.transpose() * A).inverse();
    M *= A.transpose();
    Eigen::VectorXd X = M * B;
    p1.x = X(0);
    p1.y = X(1);
}

// Compute p0,p1,p2 such that (p0, p1, p2) is the
// quadratic bezier B that minimizes \sum (samples[i] - B(u[i]))^2
//
// samples.size() must be > 2
//
void solveQuadraticUnconstrainedLeastSquare_(
        const std::vector<glm::vec2> & samples,
        const std::vector<double> & u,
        glm::dvec2 & p0,
        glm::dvec2 & p1,
        glm::dvec2 & p2)
{
    const size_t n = samples.size();
    assert(n>2);

    // Build matrices A and B such that the least square problem
    // is expressed as:
    //
    //     min || AX - B ||^2
    //
    Eigen::MatrixXd A(2*(n-2),6);
    Eigen::VectorXd B(2*(n-2));
    for(unsigned int i=1; i<n-1; i++)
    {
        const double Ui  = u[i];

        const double OneMinusUi2     = (1-Ui) * (1-Ui);
        const double TwoOneMinusUiUi = 2 * (1-Ui) * Ui;
        const double Ui2             = Ui * Ui;

        const unsigned int ix = 2*i-2;
        const unsigned int iy = 2*i-1;

        A(ix, 0) = OneMinusUi2;
        A(ix, 1) = 0;
        A(ix, 2) = TwoOneMinusUiUi;
        A(ix, 3) = 0;
        A(ix, 4) = Ui2;
        A(ix, 5) = 0;

        A(iy, 0) = 0;
        A(iy, 1) = OneMinusUi2;
        A(iy, 2) = 0;
        A(iy, 3) = TwoOneMinusUiUi;
        A(iy, 4) = 0;
        A(iy, 5) = Ui2;

        B(ix) = samples[i].x;
        B(iy) = samples[i].y;
    }

    // Solving
    //     min || AX - B ||^2
    // Is equivalent to solving
    //     At A X - At B = 0
    // Therefore:
    //     X = (At A)^(-1) At B
    Eigen::MatrixXd M = (A.transpose() * A).inverse();
    M *= A.transpose();
    Eigen::VectorXd X = M * B;
    p0.x = X(0);
    p0.y = X(1);
    p1.x = X(2);
    p1.y = X(3);
    p2.x = X(4);
    p2.y = X(5);
}

}

VecFitter::VecFitter(const std::vector<glm::vec2> & samples)
{
    const size_t n = samples.size();
    const glm::dvec2 zero(0.0, 0.0);

    // Initialize uis
    uis_.resize(n);
    if (n > 0)
    {
        uis_[n-1] = 1.0;
        uis_[0] = 0.0;
    }
    if (n > 2)
    {
        // compute arclengths
        uis_[0] = 0.0;
        for (unsigned int i=1; i<n; i++)
        {
            uis_[i] += uis_[i-1] + glm::length(samples[i] - samples[i-1]);
        }

        // divide by length
        const double length = uis_[n-1];
        if (length > 1e-10)
        {
            for (unsigned int i=1; i<n; i++)
            {
                uis_[i] /= length;
            }
        }
    }

    // Compute fit
    if (n == 0) // Constant to zero
    {
        a_ = zero;
        b_ = zero;
        c_ = zero;
        d_ = zero;
    }
    else if (n == 1) // Constant to samples[0]
    {
        a_ = samples[0];
        b_ = zero;
        c_ = zero;
        d_ = zero;
    }
    else if (n == 2) // Linear segment
    {
        a_ = samples[0];
        b_ = samples[1] - samples[0];
        c_ = zero;
        d_ = zero;
    }
    else if (n == 3) // Quadratic bezier
    {
        setFromQuadraticBezier_(samples[0], samples[1], samples[2]);
    }
    else if (n == 4) // Cubic bezier
    {
        setFromCubicBezier_(samples[0], samples[1], samples[2], samples[3]);
    }
    /*
    else if (n == 5) // Cubic bezier, with the 4 control points
                     // samples[0], samples[1], samples[2], and samples[3]
    {
        setFromBezier_(samples[0], samples[1], samples[3], samples[4]);
    }
    */
    else // Bezier fitting
    {
        // Fit a quadratic or a cubic bezier?
        enum class FitOrder { Quadratic, Cubic };
        const FitOrder fitOrder = FitOrder::Quadratic;

        // Number of Newton-Rhapson iterations
        int numNewtonIterations = 3;

        // Initialize temp vector to re-parameterize during the newton step
        std::vector<double> uis2;
        if (numNewtonIterations > 1)
        {
            uis2.resize(n);
            uis2[0] = 0.0;
            uis2[n-1] = 1.0;
        }

        // Iterate
        for (int k=0; k<numNewtonIterations; ++k)
        {
            if (fitOrder == FitOrder::Cubic)
            {
                // Solve cubic least square
                glm::dvec2 p0 = samples[0];
                glm::dvec2 p1;
                glm::dvec2 p2;
                glm::dvec2 p3 = samples[n-1];
                solveCubicLeastSquare_(samples, uis_, p0, p1, p2, p3);

                // Create cubic polynomial coefficient from bezier points
                setFromCubicBezier_(p0, p1, p2, p3);
            }
            else if (fitOrder == FitOrder::Quadratic)
            {
                // Solve quadratic least square
                glm::dvec2 p0 = samples[0];
                glm::dvec2 p1;
                glm::dvec2 p2 = samples[n-1];
                solveQuadraticLeastSquare_(samples, uis_, p0, p1, p2);
                //solveQuadraticUnconstrainedLeastSquare_(samples, uis_, p0, p1, p2);

                // Create cubic polynomial coefficient from bezier points
                setFromQuadraticBezier_(p0, p1, p2);
            }

            // Update uis using a Newton-Rhapson iteration
            if (k < numNewtonIterations-1)
            {
                for(unsigned int i=1; i<n-1; i++) // or [1, n-1] if don't want to touch borders
                {
                    const double ui = uis_[i];

                    const glm::dvec2 deltai = (*this)(ui) - glm::dvec2(samples[i]);
                    const glm::dvec2 deri   = der(ui);
                    const glm::dvec2 der2i  = der2(ui);

                    const double numerator   = glm::dot(deltai, deri);
                    const double denominator = glm::dot(deri, deri) + glm::dot(deltai, der2i);

                    const double eps = 1e-6;
                    if (std::abs(denominator) > eps)
                    {
                        double correction = numerator / denominator;

                        // apply smooth limit: unsure the correction is never more than 0.1,
                        // and more or less divide by two the correction
                        correction = 0.1 * std::tanh(5.0*correction);
                        uis2[i] = ui - correction;
                    }
                    else
                    {
                        uis2[i] = ui;
                    }
                }
                swap(uis_, uis2);
            }
        }
    }
}

void VecFitter::setFromQuadraticBezier_(
        const glm::dvec2 & p0,
        const glm::dvec2 & p1,
        const glm::dvec2 & p2)
{
    a_ = p0;
    b_ = 2.0 * (p1 - p0);
    c_ = p2 - 2.0 * p1 + p0;
    d_ = glm::dvec2(0.0, 0.0);
}

void VecFitter::setFromCubicBezier_(
        const glm::dvec2 & p0,
        const glm::dvec2 & p1,
        const glm::dvec2 & p2,
        const glm::dvec2 & p3)
{
    a_ = p0;
    b_ = 3.0 * (p1 - p0);
    c_ = 3.0 * (p2 - 2.0 * p1 + p0);
    d_ = p3 + 3.0 * (p1 - p2) - p0;
}
