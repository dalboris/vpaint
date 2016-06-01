// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "VCurve.h"
#include "CubicCurve.h"
#include "Algorithms.h"

#include <glm/geometric.hpp>

namespace VGeometry
{

VCurve::VCurve(const VCurveParams & params) :
    params_(params)
{
}

void VCurve::clear()
{
    inputSamples_.clear();
    regFits_.clear();
    regPositions_.clear();
    regWidths_.clear();
    samples_.clear();
}

void VCurve::beginFit()
{
    clear();
}

void VCurve::continueFit(const VCurveInputSample & inputSample)
{
    appendInputSample_(inputSample);
    computeRegPositions_();
    computeRegWidths_();
    computeKnots_();
    computeSamples_();
}

void VCurve::endFit()
{
    // Nothing to do
}

size_t VCurve::numKnots() const
{
    return knots_.size();
}

const VCurveKnot & VCurve::knot(unsigned int i) const
{
    return knots_.at(i);
}

const std::vector<VCurveKnot> & VCurve::knots() const
{
    return knots_;
}

size_t VCurve::numSamples() const
{
    return samples_.size();
}

const VCurveSample & VCurve::sample(unsigned int i) const
{
    return samples_.at(i);
}

const std::vector<VCurveSample> & VCurve::samples() const
{
    return samples_;
}

double VCurve::length() const
{
    return numSamples() == 0 ? 0.0 : samples_.back().arclength;
}

void VCurve::appendInputSample_(const VCurveInputSample & inputSample)
{
    // Preconditions: none

    const size_t n = inputSamples_.size();

    if (n == 0)
    {
        // Always append first sample.

        inputSamples_.push_back(inputSample);
    }
    else
    {
        // Append further samples if and only if not too close from previous
        // sample. Otherwise discard it.

        const VCurveInputSample & s0 = inputSamples_[n-1];
        const VCurveInputSample & s1 = inputSample;

        const double ds =  glm::length(s1.position - s0.position);

        if (ds > 0.1*inputSample.resolution)
        {
            inputSamples_.push_back(inputSample);
        }
    }

    // Postconditions:
    //     inputSamples_.size() > 0
    //     distance between consecutive samples > 0.1*inputSample.resolution
}

void VCurve::computeRegPositions_()
{
    computeRegFits_();
    averageRegFits_();
}

void VCurve::computeRegWidths_()
{
    // Preconditions:
    //     inputSamples_.size() > 0

    const size_t n = inputSamples_.size();
    assert(n > 0);

    // Set regWidths_ size
    regWidths_.resize(n);

    // Smooth end points
    if (n > 1)
    {
        regWidths_[0]   = 0.67 * inputSamples_[0].width + 0.33 * inputSamples_[1].width;
        regWidths_[n-1] = 0.67 * inputSamples_[n-1].width   + 0.33 * inputSamples_[n-2].width;
    }
    else
    {
        regWidths_[0]   = inputSamples_[0].width;
        regWidths_[n-1] = inputSamples_[n-1].width;
    }

    // Smooth middle points
    for (unsigned int i=1; i<n-1; ++i)
    {
        regWidths_[i] =
                0.25 * inputSamples_[i-1].width +
                0.50 * inputSamples_[i].width +
                0.25 * inputSamples_[i+1].width;
    }

    // Postconditions:
    //     inputSamples_.size() > 0
    //     regWidths_.size() == inputSamples_.size()
}

void VCurve::computeRegFits_()
{
    // Preconditions:
    //     inputSamples_.size() > 0
    //     distance between consecutive samples > 0.1*inputSample.resolution

    const size_t n = inputSamples_.size();
    assert(n > 0);

    const size_t maxNumSamplesPerFit = 5; // MUST be >= 3
    const size_t numSamplesPerFit    = std::min(maxNumSamplesPerFit, n);
    const size_t numFits             = n - numSamplesPerFit + 1;

    // Examples values:
    //
    //     n    numSamplesPerFit    numFits
    //
    //     1          1               1
    //     2          2               1
    //     3          3               1
    //     4          4               1
    //     5          5               1
    //     6          5               2
    //     7          5               3
    //     8          5               4
    //     9          5               5
    //

    // Allocate memory before the loop
    std::vector<glm::dvec2> samplesToFit(numSamplesPerFit);

    // Compute all fits
    regFits_.resize(numFits);
    for (unsigned int i=0; i<numFits; ++i)
    {
        for (unsigned int j=0; j<numSamplesPerFit; ++j)
        {
            samplesToFit[j] = inputSamples_[i+j].position;
        }
        regFits_[i] = fitQuadratic(samplesToFit); // can't fail
    }

    // Postconditions:
    //     inputSamples_.size() > 0
    //     regFits_.size() > 0
    //     regFits_.size() <= inputSamples_.size()
    //     With numSamplesPerFit = n - numFits + 1:
    //            if n>=3 then numSamplesPerCubicFit >= 3
}

namespace
{
// non-normalized bell-shaped function, centered at 0.5:
//   at u=0   : w=0 and w'=0
//   at u=0.5 : w>0 and w'=0
//   at u=1   : w=0 and w'=0
template <typename T>
inline T w_(T u) { return u*u*(1-u)*(1-u); }
}

void VCurve::averageRegFits_()
{
    // Preconditions:
    //     inputSamples_.size() > 0
    //     regFits_.size() > 0
    //     regFits_.size() <= inputSamples_.size()
    //     With numSamplesPerFit = n - numFits + 1:
    //            if n>=3 then numSamplesPerCubicFit >= 3

    const size_t n                = inputSamples_.size();
    const size_t numFits          = regFits_.size();
    const size_t numSamplesPerFit = n - numFits + 1;
    assert(n > 0);
    assert(numFits > 0);
    assert(numFits <= n);
    assert(numSamplesPerFit >= 3 || n <= 2);

    regPositions_.resize(n);
    regPositions_[0] = inputSamples_[0].position;
    for (unsigned int i=1; i<n-1; ++i)          // i = global index of sample
    {
        glm::dvec2 pos(0.0, 0.0);
        double sumW = 0.0;

        for (unsigned int j=1; j<numSamplesPerFit-1; ++j) // j = index of sample w.r.t fitter
            // loop range equivalent to do j in [0, numSamplesToFit)
            // since w_(uj) = 0.0 for j = 0 and j = numSamplesToFit-1
        {
            const int k = (int)i - (int)j; // k = index of fitter whose j-th sample is samples[i]
            if (0 <= k && k < (int)numFits)
            {
                const CubicCurve & cubicFit = regFits_[k];
                const double uj = (double) j / (double) (numSamplesPerFit-1);

                const glm::dvec2 posj = cubicFit.pos(uj);
                const double     wj   = w_(uj);

                pos  += wj * posj;
                sumW += wj;
            }
        }
        regPositions_[i] = (1/sumW) * pos;
    }
    regPositions_[n-1] = inputSamples_[n-1].position;

    // Postconditions:
    //     inputSamples_.size() > 0
    //     regWidths_.size() == inputSamples_.size()
    //
    // Note: after this averaging, we may have duplicated positions,
    // we don't remove them here to ensure reg.size() == input.size()
}


void VCurve::computeKnots_()
{
    // Preconditions:
    //     inputSamples_.size() > 0
    //     regPositions_.size() == inputSamples_.size()
    //     regWidths_.size() == inputSamples_.size()
    //
    // Note: may have duplicate positions.

    const size_t n  = inputSamples_.size();
    const size_t np = regPositions_.size();
    const size_t nw = regWidths_.size();
    assert(n > 0);
    assert(np == n);
    assert(nw == n);

    // ---- Set knot positions and widths (removing duplicates) ----

    const double resolution = inputSamples_[0].resolution;

    // Reserve memory for knots
    knots_.clear();
    knots_.reserve(n);

    // Reserve memory for distances between knots
    // d[i] = distance(knots_[i], knots_[i+1])
    std::vector<double> d;
    d.reserve(n);

    // First knot
    VCurveKnot knot;
    knot.position = regPositions_[0];
    knot.width    = regWidths_[0];
    knots_.push_back(knot);

    // Other knots
    for (unsigned int i=1; i<n; ++i)
    {
        const glm::dvec2 & p0 = knot.position;
        const glm::dvec2 & p1 = regPositions_[i];

        const double ds = glm::length(p1 - p0);

        if (ds > 0.1*resolution)
        {
            knot.position = p1;
            knot.width    = regWidths_[i];
            knots_.push_back(knot);
            d.push_back(ds);
        }
    }

    // Get number of knots after removing duplicates
    const size_t m  = knots_.size();
    assert(m > 0);

    // ---------------   Merge nearby knots   ---------------

    // Note: this is different from removing duplicates, and can't be done in
    // the same step. Removing duplicates removes knots which are *exact
    // duplicates* (up to numerical precisions).
    //
    // Here, we know that those exact duplicates (up to numerical precision)
    // are removed, and therefore that angles can be reliably computed.
    //
    // This step does the following:
    //
    //               B     C                 B or C
    //                o---o                   o
    //               /    |                  /|
    //              /     |                 / |
    //             /      |          =>    /  |
    //            /       |               /   |
    //           /        |              /    |
    //        A o         o D         A o     o D
    //
    // To ensure that something that looks like a corner *really* looks like
    // a corner.
    //
    // The criteria we use to detect those (B,C) knots to merge is:
    //
    //      (r*BC < AB) and (r*BC < CD)   with r = 2.1 (IMPORTANT: r > 2)
    //
    // In which case we delete the one with the smallest supplementary angle.
    //
    // Examples:
    //
    //               B     C
    //                o---o
    //               /    |
    //              /     |        => We delete B
    //             /      |
    //            /       |
    //           /        |
    //        A o         o D
    //
    //                B
    //                o
    //               /|
    //              / o C         => We delete C
    //             /  |
    //            /   |
    //           /    |
    //        A o     o D
    //
    //                C
    //                o
    //               /|
    //            B o |           => We delete C
    //             /  |
    //            /   |
    //           /    |
    //        A o     o D
    //
    //                C
    //                o
    //               /|
    //            B o o D         => We delete none (criteria not met)
    //             /  |
    //            /   |
    //           /    |
    //        A o     o E
    //
    // The reason we use r=2.1 is that it has to be > 2 to guarantee that the
    // algorithm doesn't create duplicate consecutives knots. And the smaller
    // r, the better (more corners detected), so r = 2.1 is a good compromise.
    //

    // Compute knot angles. By 'angle', we mean 'supplementary angle', i.e.
    // three aligned knots form an angle = 0.
    //
    // By convention, angle = 0 for end knots.
    //
    knots_[0].angle = 0.0;
    for (unsigned int i=1; i<m-1; ++i)
    {
        VCurveKnot & k0 = knots_[i-1];
        VCurveKnot & k1 = knots_[i];
        VCurveKnot & k2 = knots_[i+1];

        k1.angle = computeSupplementaryAngle(k0.position, k1.position, k2.position);
    }
    knots_[m-1].angle = 0.0;

    // Merge knots in-place. Don't touch first knot
    //
    const double r = 2.1;
    unsigned int i1 = 0; // i1: index of knot in old list
    unsigned int i2 = 0; // i2: index of knot in new list
    while (i1+3 < m) // i1 < m-3, but we avoiding the difference because m-3
                     // could wraps around to a very large unsigned int.
    {
        // Increment indices.
        // First loop iteration has i1 = i2 = 1
        // Last loop iteration has i1 = m-3
        ++i1;
        ++i2;

        // Notations:
        //   VCurveKnot & A = knots_[i1-1];
        //   VCurveKnot & B = knots_[i1];
        //   VCurveKnot & C = knots_[i1+1];
        //   VCurveKnot & D = knots_[i1+2];
        //
        // Considered "before merging". Note that at this point, A may have
        // been overriden during a previous iteration, by B, C, and D are still
        // untouched.
        //
        // B and C are the two knots that we are considering to merge.
        //
        VCurveKnot & B = knots_[i1];
        VCurveKnot & C = knots_[i1+1];

        // Get distances between knots. Those distances must be distances
        // between the previous, unmerged knots. So we can't do
        // glm::length(B.position - A.position), because A may have been
        // overidden already.
        //
        const double AB = d[i1-1];
        const double BC = d[i1];
        const double CD = d[i1+1];

        // Test merge criteria
        //
        if ((r*BC < AB) && (r*BC < CD))
        {
            // Merge BC into (B or C)

            const double angleB = B.angle;
            const double angleC = C.angle;

            if (angleB < angleC)
            {
                // Merge BC into C
                 knots_[i2] = C;

                 // Increment i1 (but not i2)
                 ++i1;
            }
            else
            {
                // Merge BC into B
                knots_[i2] = B;

                // Increment i1 (but not i2)
                ++i1;
            }
        }
        else
        {
            // Don't merge (i.e., don't increment i1)
            knots_[i2] = B;
        }
    }
    // copy the last knot, or the last two knots (depending
    // whether the last loop iteration was a merge or not)
    while (i1 < m-1)
    {
        ++i1;
        ++i2;
        knots_[i2] = knots_[i1];
    }

    // Get number of knots after merging nearby knots
    const size_t p = i2 + 1;
    assert(p > 0);
    assert(p <= m);
    knots_.resize(p);

    // Recompute angles
    knots_[0].angle = 0.0;
    for (unsigned int i=1; i<p-1; ++i)
    {
        VCurveKnot & k0 = knots_[i-1];
        VCurveKnot & k1 = knots_[i];
        VCurveKnot & k2 = knots_[i+1];

        k1.angle = computeSupplementaryAngle(k0.position, k1.position, k2.position);
    }
    knots_[p-1].angle = 0.0;


    // ---------------   Decide which knots are corner knots   ---------------

    knots_[0].isCorner = true;
    for (unsigned int i=1; i<p-1; ++i)
    {
        VCurveKnot & k1 = knots_[i];

        k1.isCorner = (k1.angle > params_.maxSmoothKnotAngle);
    }
    knots_[p-1].isCorner = true;

    // Postconditions:
    //     knots_.size() > 0
    //     consecutive knots have a distance > 0.1*eps (very unlikely worst case)
    //
}

void VCurve::computeSamples_()
{
    const size_t n = knots_.size();
    assert(n>0);
    assert(knots_[0].isCorner);
    assert(knots_[n-1].isCorner);

    const unsigned int numSubdivisionSteps = 3;
    const double w = 1.0 / 16.0; // tension parameter for 4-point scheme

    samples_.clear();

    // Create all but last sample
    for(unsigned int i=0; i<n-1; ++i)
    {
        // In this loop, we create the samples between knots_[i] and
        // knots_[i+1]. For this, we also need to use the two previous knots
        // and the two following knots, unless there are corner knots.
        //
        // More precisely, to compute the samples between knots_[i] and
        // knots_[i+1] we need the knots at i-2, i-1, i, i+1, i+2, and i+3, but
        // saturating the indices at corner knots. We call these indices i0,
        // i1, i2, i3, i4, and i5. We call the corresponding knots k0, k1, k2,
        // k3, k4, and k5. (with possibly, k0 = k2, k1 = k2, k4 = k3, and/or k5
        // = k3).

        // Get knots at i and i+1
        const unsigned int i2 = i;
        const unsigned int i3 = i+1;
        const VCurveKnot & k2 = knots_[i2];
        const VCurveKnot & k3 = knots_[i3];

        // Get knot at "i-1"
        const unsigned int i1 = k2.isCorner ? i2 : i2 - 1;
        const VCurveKnot & k1 = knots_[i1];

        // Get knot at "i-2"
        const unsigned int i0 = k1.isCorner ? i1 : i1 - 1;
        const VCurveKnot & k0 = knots_[i0];

        // Get knot at "i+2"
        const unsigned int i4 = k3.isCorner ? i3 : i3 + 1;
        const VCurveKnot & k4 = knots_[i4];

        // Get knot at "i+3"
        const unsigned int i5 = k4.isCorner ? i4 : i4 + 1;
        const VCurveKnot & k5 = knots_[i5];

        // Subdivide recursively the curve between k2 and k3. The ASCII art
        // represents what's in the vectors. The empty values in the vectors
        // are to avoid special treatment for the first iteration, or when
        // there's no iteration.

        // Initialize vectors:
        //
        //     |   |   | 0 | 1 | 2 | 3 | 4 | 5 |   |   |
        //
        std::vector<glm::dvec2> positions(10);
        positions[2] = k0.position;
        positions[3] = k1.position;
        positions[4] = k2.position;
        positions[5] = k3.position;
        positions[6] = k4.position;
        positions[7] = k5.position;
        //
        std::vector<double> widths(10);
        widths[2] = k0.width;
        widths[3] = k1.width;
        widths[4] = k2.width;
        widths[5] = k3.width;
        widths[6] = k4.width;
        widths[7] = k5.width;

        for (unsigned int j=0; j<numSubdivisionSteps; ++j)
        {
            // Meta-comment: ASCII art and values are for first iteration

            const size_t p    = positions.size() - 4; // == 6

            // Allocate memory for storing result of iteration
            std::vector<glm::dvec2> newPositions(2*p-1); // == 11
            std::vector<double> newWidths(2*p-1);

            // Spread out values
            //
            //   old:       |   |   | 0 | 1 | 2 | 3 | 4 | 5 |   |   |
            //
            //   new:       | 0 |   | 1 |   | 2 |   | 3 |   | 4 |   | 5 |
            //
            for (unsigned int k=0; k<p; ++k) // k in [0..5]
            {
                newPositions[2*k] = positions[k+2];
                newWidths[2*k] = widths[k+2];
            }

            // Compute useful interpolated values based on 4 values around.
            //
            //   after i=0:  | 0 |   | 1 | 6 | 2 |   | 3 |   | 4 |   | 5 |
            //
            //   after i=1:  | 0 |   | 1 | 6 | 2 | 7 | 3 |   | 4 |   | 5 |
            //
            //   after i=2:  | 0 |   | 1 | 6 | 2 | 7 | 3 | 8 | 4 |   | 5 |
            //
            for (unsigned int k=0; k<p-3; ++k) // i in [0..2]
            {
                const unsigned int k1  = 2*k;    // in
                const unsigned int k2  = k1 + 2; // in
                const unsigned int k25 = k1 + 3; // out
                const unsigned int k3  = k1 + 4; // in
                const unsigned int k4  = k1 + 6; // in

                newPositions[k25] = interpolateUsingDynLevin(
                            newPositions[k1],
                            newPositions[k2],
                            newPositions[k3],
                            newPositions[k4],
                            w);

                newWidths[k25] = interpolateUsingDynLevin(
                            newWidths[k1],
                            newWidths[k2],
                            newWidths[k3],
                            newWidths[k4],
                            w);
            }

            // Swap old and new
            swap(positions, newPositions);
            swap(widths, newWidths);
        }

        // Here is how it looks after three iterations:
        //
        //     |    |    | 0  | 1  | 2  | 3  | 4  | 5  |    |    |
        //
        //     | 0  |    | 1  | 6  | 2  | 7  | 3  | 8  | 4  |    | 5  |
        //
        //     | 1  |    | 6  | 9  | 2  | 10 | 7  | 11 | 3  | 12 | 8  |    | 4  |
        //
        //     | 6  |    | 9  | 13 | 2  | 14 | 10 | 15 | 7  | 16 | 11 | 17 | 3  | 18 | 12 |    | 8  |
        //                         \____________________________________________/
        //                        here are our samples between the knots k2 and k3 (both included)

        // Make samples from k2 (included) to k3 (not included)
        for (unsigned int k=4; k<positions.size()-5; ++k)
        {
            // Sample to add to samples_
            VCurveSample sample;

            // Set position and width
            sample.position = positions[k];
            sample.width = widths[k];

            // Append sample
            samples_.push_back(sample);
        }
    }

    // Create last sample
    VCurveSample lastSample;
    lastSample.position = knots_[n-1].position;
    lastSample.width = knots_[n-1].width;
    samples_.push_back(lastSample);

    // Now, we should have at least one sample
    const size_t m = samples_.size();
    assert(m > 0);

    // Compute arclengths
    samples_[0].arclength = 0.0;
    for (unsigned int i=1; i<m; ++i)
    {
        VCurveSample & s0 = samples_[i-1];
        VCurveSample & s1 = samples_[i];

        const glm::dvec2 dp = s1.position - s0.position;
        const double ds = glm::length(dp);

        s1.arclength = s0.arclength + ds;
    }

    // Compute tangents and normals
    for (unsigned int i=0; i<m; ++i)
    {
        // Get samples before, at, and after i
        VCurveSample & s0 = samples_[i>0 ? i-1 : 0];
        VCurveSample & s1 = samples_[i];
        VCurveSample & s2 = samples_[i<m-1 ? i+1 : m-1];

        // Compute difference between the sample before and the sample after
        const glm::dvec2 dp = s2.position - s0.position;
        const double ds = glm::length(dp);

        // Compute tangent
        if (ds > 1e-6)
            s1.tangent = dp / ds;
        else
            s1.tangent = glm::dvec2(1.0, 0.0);

        // Compute normal
        s1.normal.x = - s1.tangent.y;
        s1.normal.y = + s1.tangent.x;
    }
}

} // end namespace VGeometry
