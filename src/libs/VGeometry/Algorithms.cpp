// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "Algorithms.h"

#include <glm/geometric.hpp>
#include <Eigen/Dense>

namespace VGeometry
{

Eigen::VectorXd solveLinearLeastSquareUsingNormalEquations(
        const Eigen::MatrixXd & A,
        const Eigen::VectorXd & b)
{
    return (A.transpose() * A).ldlt().solve(A.transpose() * b);
}

CubicCurve solveCubicWithGivenEndPoints(
            const glm::dvec2 & startPos,
            const glm::dvec2 & endPos,
            const glm::dvec2 * pos,
            const double * u,
            size_t n)
{
    assert(n >= 2);

    // Express linear least square as min || Ax - b ||^2
    Eigen::MatrixXd A(2*n,4);
    Eigen::VectorXd b(2*n);
    for(unsigned int i=0; i<n; ++i)
    {
        const double Ui  = u[i];
        const double Ui2 = Ui*Ui;
        const double Ui3 = Ui2*Ui;

        const double OneMinusUi  = 1 - Ui;
        const double OneMinusUi2 = OneMinusUi*OneMinusUi;
        const double OneMinusUi3 = OneMinusUi2*OneMinusUi;

        const double ThreeOneMinusUi2Ui = 3 * OneMinusUi2 * Ui;
        const double ThreeOneMinusUiUi2 = 3 * OneMinusUi * Ui2;

        const unsigned int ix = 2*i;
        const unsigned int iy = 2*i+1;

        A(ix, 0) = ThreeOneMinusUi2Ui;
        A(ix, 1) = 0;
        A(ix, 2) = ThreeOneMinusUiUi2;
        A(ix, 3) = 0;

        A(iy, 0) = 0;
        A(iy, 1) = ThreeOneMinusUi2Ui;
        A(iy, 2) = 0;
        A(iy, 3) = ThreeOneMinusUiUi2;

        b(ix) = pos[i].x - OneMinusUi3 * startPos.x - Ui3 * endPos.x;
        b(iy) = pos[i].y - OneMinusUi3 * startPos.y - Ui3 * endPos.y;
    }

    // Solve the linear least square problem
    Eigen::VectorXd x = solveLinearLeastSquareUsingNormalEquations(A, b);
    glm::dvec2 p1(x(0), x(1));
    glm::dvec2 p2(x(2), x(3));

    // Return cubic curve
    return CubicCurve::fromBezier(startPos, p1, p2, endPos);
}

CubicCurve solveCubicWithGivenEndPoints(
        const glm::dvec2 & startPos,
        const glm::dvec2 & endPos,
        const std::vector<glm::dvec2> & pos,
        const std::vector<double> & u)
{
    assert(pos.size() == u.size());

    return solveCubicWithGivenEndPoints(
                startPos,
                endPos,
                pos.data(),
                u.data(),
                pos.size());
}

QuadraticCurve solveQuadraticWithGivenEndPoints(
        const glm::dvec2 & startPos,
        const glm::dvec2 & endPos,
        const glm::dvec2 * pos,
        const double * u,
        size_t n)
{
    assert(n >= 1);

    // Express linear least square as min || Ax - b ||^2
    Eigen::MatrixXd A(2*n,2);
    Eigen::VectorXd b(2*n);
    for(unsigned int i=0; i<n; ++i)
    {
        const double Ui  = u[i];
        const double Ui2 = Ui*Ui;
        const double TwoOneMinusUiUi = 2 * (1-Ui) * Ui;
        const double OneMinusUi2 = (1-Ui) * (1-Ui);

        const unsigned int ix = 2*i;
        const unsigned int iy = 2*i+1;

        A(ix, 0) = TwoOneMinusUiUi;
        A(ix, 1) = 0;

        A(iy, 0) = 0;
        A(iy, 1) = TwoOneMinusUiUi;

        b(ix) = pos[i].x - OneMinusUi2 * startPos.x - Ui2 * endPos.x;
        b(iy) = pos[i].y - OneMinusUi2 * startPos.y - Ui2 * endPos.y;
    }

    // Solve the linear least square problem
    Eigen::VectorXd x = solveLinearLeastSquareUsingNormalEquations(A, b);
    glm::dvec2 p1(x(0), x(1));

    // Return quadratic curve
    return QuadraticCurve::fromBezier(startPos, p1, endPos);
}

QuadraticCurve solveQuadraticWithGivenEndPoints(
        const glm::dvec2 & startPos,
        const glm::dvec2 & endPos,
        const std::vector<glm::dvec2> & pos,
        const std::vector<double> & u)
{
    assert(pos.size() == u.size());

    return solveQuadraticWithGivenEndPoints(
                startPos,
                endPos,
                pos.data(),
                u.data(),
                pos.size());
}

QuadraticCurve solveQuadratic(
        const glm::dvec2 * pos,
        const double * u,
        size_t n)
{
    assert(n >= 3);

    // Express linear least square as min || Ax - b ||^2
    Eigen::MatrixXd A(2*n,6);
    Eigen::VectorXd b(2*n);
    for(unsigned int i=0; i<n; ++i)
    {
        const double Ui  = u[i];

        const double OneMinusUi2     = (1-Ui) * (1-Ui);
        const double TwoOneMinusUiUi = 2 * (1-Ui) * Ui;
        const double Ui2             = Ui * Ui;

        const unsigned int ix = 2*i;
        const unsigned int iy = 2*i+1;

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

        b(ix) = pos[i].x;
        b(iy) = pos[i].y;
    }

    // Solve the linear least square problem
    Eigen::VectorXd x = solveLinearLeastSquareUsingNormalEquations(A, b);
    glm::dvec2 p0(x(0), x(1));
    glm::dvec2 p1(x(2), x(3));
    glm::dvec2 p2(x(4), x(5));

    // Return quadratic curve
    return QuadraticCurve::fromBezier(p0, p1, p2);
}

QuadraticCurve solveQuadratic(
        const std::vector<glm::dvec2> & pos,
        const std::vector<double> & u)
{
    assert(pos.size() == u.size());

    return solveQuadratic(
                pos.data(),
                u.data(),
                pos.size());
}

namespace
{

void initializeParameterization_(
        const std::vector<glm::dvec2> & points,
        std::vector<double> & u,
        std::vector<double> & uTemp,
        unsigned int numNewtonIterations)
{
    const size_t n = points.size();

    // Initialize u, using arclength of points (considered linear by parts) as a
    // heuristic.
    //
    u.resize(n);
    u[0] = 0.0;
    for (unsigned int i=1; i<n; i++)
    {
        u[i] += u[i-1] + glm::length(points[i] - points[i-1]);
    }
    const double length = u[n-1];
    if (length > 1e-10)
    {
        for (unsigned int i=1; i<n; i++)
        {
            u[i] /= length;
        }
    }

    // Initialize uTemp (temporary vector to be swapped with u after
    // reparameterization). We only perform this if numNewtonIterations > 1,
    // since otherwise, there are no reparameterization steps, and therefore
    // uTemp is not used.
    //
    if (numNewtonIterations > 1)
    {
        uTemp.resize(n);
        uTemp[0] = 0.0;
        uTemp[n-1] = 1.0;
    }
}

template <class PolynomialCurve>
void reparameterizeUsingNewtonRhapson_(
        const std::vector<glm::dvec2> & points,
        const PolynomialCurve & currentFit,
        std::vector<double> & u,
        std::vector<double> & uTemp)
{
    const size_t n = points.size();

    const double inv2ClampValue = 0.5 * n;
    const double clampValue = 0.5 / inv2ClampValue;

    for(unsigned int i=1; i<n-1; i++)
    {
        const double ui = u[i];

        const glm::dvec2 deltai = currentFit.pos(ui) - glm::dvec2(points[i]);
        const glm::dvec2 deri   = currentFit.der(ui);
        const glm::dvec2 der2i  = currentFit.der2(ui);

        const double numerator   = glm::dot(deltai, deri);
        const double denominator = glm::dot(deri, deri) + glm::dot(deltai, der2i);

        const double eps = 1e-10;
        if (std::abs(denominator) > eps)
        {
            double correction = numerator / denominator;

            // Clamp the correction smoothly. For instance, if
            // n=10, then it never makes sense to change u by more
            // than 0.1. A correction > 0.1 would mean the initial
            // fitting if bad, but applying such a huge correction
            // is likely to make things even worse.
            //
            correction = clampValue * std::tanh(inv2ClampValue*correction);

            // Apply correction
            uTemp[i] = ui - correction;
        }
        else
        {
            uTemp[i] = ui;
        }
    }
    swap(u, uTemp);
}

}

QuadraticCurve fitQuadratic(const std::vector<glm::dvec2> & points)
{
    const size_t n = points.size();
    const glm::dvec2 zero(0.0, 0.0);

    if (n == 0)
    {
        // Constant to zero
        return QuadraticCurve(zero, zero, zero);
    }
    else if (n == 1)
    {
        // Constant to samples[0]
        return QuadraticCurve(points[0], zero, zero);
    }
    else if (n == 2)
    {
        // Linear between samples[0] and samples[1]
        return QuadraticCurve(points[0], points[1] - points[0], zero);
    }
    else if (n == 3)
    {
        // Quadratic bezier
        return QuadraticCurve::fromBezier(points[0], points[1], points[2]);
    }
    else
    {
        // Number of Newton-Rhapson iterations
        const int numNewtonIterations = 3;

        // Iterate
        QuadraticCurve res;
        std::vector<double> u;
        std::vector<double> uTemp;
        for (int k=0; k<numNewtonIterations; ++k)
        {
            // Compute parameterization
            if (k == 0)
                initializeParameterization_(points, u, uTemp, numNewtonIterations);
            else
                reparameterizeUsingNewtonRhapson_(points, res, u, uTemp);

            // Solve linear least square system with given parameterization
            res = solveQuadraticWithGivenEndPoints(
                    points[0],   // startPos
                    points[n-1], // endPos
                    &points[1],  // pos
                    &u[1],       // u
                    n-2);        // n
        }

        // Return quadratic Bezier fitting
        return res;
    }
}

CubicCurve fitCubic(const std::vector<glm::dvec2> & points)
{
    const size_t n = points.size();

    if (n < 4)
    {
        // Constant, linear, or quadratic bezier
        return fitQuadratic(points);
    }
    else if (n == 4)
    {
        // Cubic bezier
        return CubicCurve::fromBezier(points[0], points[1], points[2], points[3]);
    }
    else
    {
        // Number of Newton-Rhapson iterations
        const int numNewtonIterations = 3;

        // Iterate
        CubicCurve res;
        std::vector<double> u;
        std::vector<double> uTemp;
        for (int k=0; k<numNewtonIterations; ++k)
        {
            // Compute parameterization
            if (k == 0)
                initializeParameterization_(points, u, uTemp, numNewtonIterations);
            else
                reparameterizeUsingNewtonRhapson_(points, res, u, uTemp);

            // Solve linear least square system with given parameterization
            res = solveCubicWithGivenEndPoints(
                    points[0],   // startPos
                    points[n-1], // endPos
                    &points[1],  // pos
                    &u[1],       // u
                    n-2);        // n
        }

        // Return cubic Bezier fitting
        return res;
    }
}

} // end namespace VGeometry
