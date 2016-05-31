// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef VGEOMETRY_ALGORITHMS_H
#define VGEOMETRY_ALGORITHMS_H

#include "QuadraticCurve.h"
#include "CubicCurve.h"

#include <Eigen/Core>
#include <glm/vec2.hpp>
#include <vector>
#include <cmath>

namespace VGeometry
{

/// Solve the given linear least square problem:
///
///     min || Ax - b ||^2
///
/// Using normal equations. This is the fastest but least accurate way to
/// solve a linear least square problem. See
/// https://eigen.tuxfamily.org/dox-devel/group__LeastSquares.html
/// for more information.
///
Eigen::VectorXd solveLinearLeastSquareUsingNormalEquations(
        const Eigen::MatrixXd & A,
        const Eigen::VectorXd & b);

/// Computes the unique CubicCurve \p c that satisfies c.pos(0) = startPos,
/// c.pos(1) = endPos, and minimizes the least square distance:
///
///     \sum_{i in [0..n-1]} ( pos[i] - c.pos(u[i]) )^2
///
/// pos and u must be C-style arrays of size n >= 2 (if n < 2, then
/// the linear system to solve is underdetermined and has an infinite number of
/// solutions, unless the u[i] are not all different).
///
/// The values u[i] must be all different, otherwise, the linear system may
/// have no solutions.
///
/// You must not include startPos and endPos in pos and u.
///
/// This is a low-level function that may lead to overfitting artefacts,
/// especially when the number of samples is low. Prefer using the higher-level
/// method fitCubic() which is more robust, computes appropriates u for you,
/// and uses heuristics to avoid overfitting.
///
CubicCurve solveCubicWithGivenEndPoints(
        const glm::dvec2 & startPos,
        const glm::dvec2 & endPos,
        const glm::dvec2 * pos,
        const double * u,
        size_t n);

/// Convenient overload of solveCubicWithGivenEndPoints()
///
CubicCurve solveCubicWithGivenEndPoints(
        const glm::dvec2 & startPos,
        const glm::dvec2 & endPos,
        const std::vector<glm::dvec2> & pos,
        const std::vector<double> & u);

/// Computes the unique QuadraticCurve \p c that satisfies c.pos(0) = startPos,
/// c.pos(1) = endPos, and minimizes the least square distance:
///
///     \sum_{i in [0..n-1]} ( pos[i] - c.pos(u[i]) )^2
///
/// where n = samples.size().
///
/// pos and u must be C-style arrays of size n >= 1 (if n < 1, then
/// the linear system to solve is underdetermined and has an infinite number of
/// solutions, unless the u[i] are not all different).
///
/// The values u[i] must be all different, otherwise, the linear system may
/// have no solutions.
///
/// You must not include startPos and endPos in pos and u.
///
/// This is a low-level function that may lead to overfitting artefacts,
/// especially when the number of samples is low. Prefer using the higher-level
/// method fitQuadratic() which is more robust, computes appropriates u for you,
/// and uses heuristics to avoid overfitting.
///
QuadraticCurve solveQuadraticWithGivenEndPoints(
        const glm::dvec2 & startPos,
        const glm::dvec2 & endPos,
        const glm::dvec2 * pos,
        const double * u,
        size_t n);

/// Convenient overload of solveQuadraticWithGivenEndPoints().
///
QuadraticCurve solveQuadraticWithGivenEndPoints(
        const glm::dvec2 & startPos,
        const glm::dvec2 & endPos,
        const std::vector<glm::dvec2> & pos,
        const std::vector<double> & u);

/// Computes a QuadraticCurve \p c that minimizes the least square distance:
///
///     \sum_{i in [0..n-1]} ( pos[i] - c.pos(u[i]) )^2
///
/// where n = samples.size().
///
/// The std::vectors pos and u must have the same size n >= 3 (if n < 3, then
/// the linear system to solve is underdetermined and has an infinite number of
/// solutions, unless the u[i] are not all different).
///
/// The values u[i] must be all different, otherwise, the linear system may
/// have no solutions.
///
/// This is a low-level function that may lead to overfitting artefacts,
/// especially when the number of samples is low. Prefer using the higher-level
/// method fitQuadratic() which is more robust, computes appropriates u for you,
/// and uses heuristics to avoid overfitting.
///
QuadraticCurve solveQuadratic(
        const glm::dvec2 * pos,
        const double * u,
        size_t n);

/// Convenient overload of solveQuadratic().
///
QuadraticCurve solveQuadratic(
        const std::vector<glm::dvec2> & pos,
        const std::vector<double> & u);

/// Computes a QuadraticCurve that approximates the given points, using heuristics
/// for small number of points, and minimizing the least square distance for
/// big enough number of points.
///
QuadraticCurve fitQuadratic(
        const std::vector<glm::dvec2> & points);

/// Computes a CubicCurve that approximates the given points, using heuristics
/// for small number of points, and minimizing the least square distance for
/// big enough number of points.
///
CubicCurve fitCubic(
        const std::vector<glm::dvec2> & points);

/// Returns the point corresponding to one subdivision step using the Dyn-Levin
/// 4-point subdivision scheme with a tension parameter of w.
///
/// Scalar = float or double
///
template <class Point, class Scalar>
Point interpolateUsingDynLevin(
        const Point & p0,
        const Point & p1,
        const Point & p2,
        const Point & p3,
        Scalar w)
{
    return (0.5f + w) * (p1 + p2) - w * (p0 + p3);
}

/// Returns the oriented angle between three glm::dvec2, in [-pi, pi].
/// If the three points are colinear, then angle is either pi ot -pi.
///
///                               p1        p2
///        angle > 0   o p2       o---------o
///                __ /          /__/
///               /  /          /    angle < 0
///        o--------o          o
///       p0        p1         p0
///
///
double computeOrientedAngle(const glm::dvec2 & p0,
                            const glm::dvec2 & p1,
                            const glm::dvec2 & p2);

/// Returns the non-oriented angle between three glm::dvec2, in [0, pi].
/// If the three points are colinear, then angle is pi.
///
/// This is equivalent to std::abs(computeOrientedAngle(...))
///
double computeAngle(const glm::dvec2 & p0,
                        const glm::dvec2 & p1,
                        const glm::dvec2 & p2);

/// Returns the non-oriented supplementary angle between three glm::dvec2, in [0, pi].
/// If the three points are colinear, then angle is 0.
///
/// This is equivalent to PI - computeAngle(...)
///
double computeSupplementaryAngle(const glm::dvec2 & p0,
                                 const glm::dvec2 & p1,
                                 const glm::dvec2 & p2);

} // end namespace VGeometry

#endif // VGEOMETRY_ALGORITHMS_H
