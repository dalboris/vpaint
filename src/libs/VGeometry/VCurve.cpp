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

    assert(inputSamples_.size() > 0);
}

void VCurve::computeRegPositions_()
{
    computeRegFits_();
    averageRegFits_();
}

// Note on preconditions and postconditions:
//
// There are two types of guarantees:
//
//    - Integer Guarantees (I): in general, those are vector sizes.
//      those are hard-checked with assert() since we want to
//      be really sure that we are not accessing memory that we
//      shouldn't
//
//   - Floating Point guarantees (FP): in general, those are distances
//     between consecutive values in vector, guaranteed (or not)
//     to be greater than some eps. Those are not checked by
//     assert() because it would make the code less readable,
//     (and execution slower: assert may not be removed even in
//      release builds).
//
// If a floating point guarantee should infer an integer guarantee,
// then we call this a "Loose Integer guarantee".
//
// Example:
//
// \code
// Preconditions:
//    (I)  v1.size() >= 2
//    (FP) distance(v1[i], v1[i+1]) > eps, for all i
//
// const size_t n = v1.size();
// assert(n >= 2);
//
// v2.clear();
// v2.push_back(v1[0]);
// for (unsigned int i=1; i<n; ++i)
// {
//     if (distance(v2.back(), v1[i]) > eps)
//     {
//         v2.push_back(v1[i]);
//     }
// }
//
// Midconditions:
//    (I)  v1.size() >= 2
//    (I)  v2.size() >= 1
//    (LI) v2.size() >= 2
//    (FP) distance(v1[i], v1[i+1]) > eps, for all i
//    (FP) distance(v2[i], v2[i+1]) > eps, for all i
//
// if (v2.size() == 1)
// {
//     v2.push_back(v1.back());
// }
//
// Postconditions:
//    (I)  v1.size() >= 2
//    (I)  v2.size() >= 2
//    (FP) distance(v1[i], v1[i+1]) > eps, for all i
//    (FP) distance(v2[i], v2[i+1]) > eps, for all i
//
// assert(v1.size() >= 2);
// assert(v2.size() >= 2);
// \endcode
//
// Integer pre-conditions and post-conditions must be checked via asserts.
//
// Floating Point conditions don't have to be checked.
//
// Loose Integer conditions must be checked via "if" (not assert), and
// corrected into a (strong) Integer condition (i.e., not relying on any
// floating point computation). When doing so, you don't *have to* enforce FP
// conditions. For instance, in the example above, when correcting the LI, we
// don't check whether adding v1.back() breaks the FP, and we just assume it
// does not. It is meaningless to enforce a FP during a LI->I correction, since
// anyway there's already something wrong with the FP, otherwise we wouldn't do
// the LI->I correction in the first place. Just try to make make something
// *sensible*, the most important being that the I conditions don't rely
// on floating point computations.
//

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

    assert(inputSamples_.size() > 0);
    assert(regWidths_.size() == inputSamples_.size());
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
    //            if n>=3 then numSamplesPerFit >= 3

    assert(inputSamples_.size() > 0);
    assert(regFits_.size() > 0);
    assert(regFits_.size() <= inputSamples_.size());
    assert(inputSamples_.size() - regFits_.size() + 1 >= 3 || inputSamples_.size() <= 2);
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
    //     regPositions_.size() == inputSamples_.size()
    //
    // Note: after this averaging, we may have duplicated positions,
    // we don't remove them here to ensure reg.size() == input.size()

    assert(inputSamples_.size() > 0);
    assert(regPositions_.size() == inputSamples_.size());
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

    const double eps  = 1e-10; // numerical precision
    const double resolution =  // resolution precision
            std::max(10 * eps,
                     inputSamples_[0].resolution);

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

        if (ds > resolution)
        {
            knot.position = p1;
            knot.width    = regWidths_[i];
            knots_.push_back(knot);
            d.push_back(ds);
        }
    }

    // Midconditions:
    //     knots_.size() > 0
    //     distance between consecutive knots > resolution
    //     distance between consecutive knots > 10 * eps

    const size_t m  = knots_.size();
    assert(m > 0);

    // ---------------   Merge nearby knots   ---------------

    // Note: this is different from removing duplicates, and can't be done in
    // the same step. Removing duplicates removes knots which are *exact
    // duplicates* (up to resolution).
    //
    // Here, we know that those exact duplicates (up to resolution)
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
    //      (r*BC < AB) and (r*BC < CD)   with r > 2
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
    //            B o |           => We delete B
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
    // IMPORTANT: we need r > 2 to guarantee that the
    // algorithm doesn't create duplicate consecutives knots.
    //
    // In the worst case, the distance between consecutive knots becomes:
    //
    //     d_min <- min( (r - 2) * d_min, d_min )
    //
    // Example, with r=2.1:
    //
    //     d_min <- 0.1 * d_min = 0.1 * (10 * eps) = eps
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
    const double r = 4;
    unsigned int i1 = 0; // i1: index of knot in old list
    unsigned int i2 = 0; // i2: index of knot in new list
    while (i1+3 < m) // same as while(i1 < m-3) but the latter causes the
                     // unsigned int to wrap to MAX_UINT for small m.
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
    while (i1+1 < m)
    {
        ++i1;
        ++i2;
        knots_[i2] = knots_[i1];
    }
    // Discards remnant knots from old list
    knots_.resize(i2 + 1);

    // Midconditions:
    //     knots_.size() > 0
    //     distance between consecutive knots > eps

    // Get number of knots after merging nearby knots
    const size_t p = knots_.size();
    assert(p > 0);
    assert(p <= m);

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
    //     consecutive knots have a distance > eps
    //                              distance > 0.1*resolution
}

namespace
{

class AdaptiveSampler
{
private:
    // Parameters
    unsigned int maxNumSubdivision;
    double maxAngle;
    double w;

    // Pre-allocated buffers where to store computed samples
    //
    // Note:
    //   With maxNumSubdivision = 6, then:
    //     maxNumSamplesPerKnot = 64
    //     sampleBufferSize = 321
    //     sizeof(vp+vw) = 321 * 3 * sizeof(double) = 7704B  => fits in L1 cache.
    //
    glm::dvec2 * vp; // buffer of sample positions
    double * vw;     // buffer of sample widths

    // Init indices
    unsigned int iA;
    unsigned int iB;
    unsigned int iC;
    unsigned int iD;
    unsigned int iE;
    unsigned int iF;
    unsigned int iFirst;
    unsigned int dFirst;

    // Indices of computed samples
    std::vector<unsigned int> sampleIndices;

public:
    AdaptiveSampler(
            unsigned int maxNumSubdivision,
            double maxAngle,
            double w) :
        maxNumSubdivision(maxNumSubdivision),
        maxAngle(maxAngle),
        w(w)
    {
        // Get buffers' size
        const unsigned int maxNumSamplesPerKnot = 1 << maxNumSubdivision;
        const size_t       sampleBufferSize     = 5*maxNumSamplesPerKnot + 1;

        // Allocate buffers
        vp = new glm::dvec2[sampleBufferSize];
        vw = new double[sampleBufferSize];

        // Compute init indices
        iA = 0;
        iB = iA + maxNumSamplesPerKnot;
        iC = iB + maxNumSamplesPerKnot;
        iD = iC + maxNumSamplesPerKnot;
        iE = iD + maxNumSamplesPerKnot;
        iF = iE + maxNumSamplesPerKnot;
        dFirst = maxNumSamplesPerKnot / 2;
        iFirst = iC + dFirst;
    }

    ~AdaptiveSampler()
    {
        // Deallocate buffers
        delete[] vp;
        delete[] vw;
    }

    size_t numSamples() const
    {
        return sampleIndices.size();
    }

    glm::dvec2 position(unsigned int j) const
    {
        return vp[sampleIndices[j]];
    }

    double width(unsigned int j) const
    {
        return vw[sampleIndices[j]];
    }

    void computeSampling(
            const VCurveKnot & A,
            const VCurveKnot & B,
            const VCurveKnot & C,
            const VCurveKnot & D,
            const VCurveKnot & E,
            const VCurveKnot & F)
    {
        // Initialize buffer positions
        vp[iA] = A.position;
        vp[iB] = B.position;
        vp[iC] = C.position;
        vp[iD] = D.position;
        vp[iE] = E.position;
        vp[iF] = F.position;

        // Initialize buffer widths
        vw[iA] = A.width;
        vw[iB] = B.width;
        vw[iC] = C.width;
        vw[iD] = D.width;
        vw[iE] = E.width;
        vw[iF] = F.width;

        // Clear indices of samples
        sampleIndices.clear();

        // Add first sample
        sampleIndices.push_back(iC);

        // Sample recursively
        recurse_(iFirst, dFirst);

        // Add last sample
        sampleIndices.push_back(iD);
    }

private:
    // Adaptive sampling recursive method.
    //
    // XXX May be worth de-recursify this.
    //
    // Note: this function sometimes re-compute already computed samples. However:
    //     - In the worth case, no more than 2*numSamples() samples are computed.
    //     - Computing a sample is super fast (everything fits in L1 cache, and
    //       interpolateUsingDynLevin() is only four additions and two multiplications)
    //     - What is expensive is computeSupplementaryAngle() which cannot be
    //       called less than it already is.
    //
    void recurse_(
            unsigned int i, // index of sample to compute
            unsigned int d) // delta index between samples at this recursion level (power of two)
    {
        // Precondition: the 6 samples around i are already computed. Example for d=2:
        //
        //  v = ... |   |i-5d|   |   |   |i-3d|   |   |   |i-d|   |   |   |i+d|   |   |   |i+3d|   |   |   |i+5d|   | ...

        // Compute sample position and width
        vp[i] = interpolateUsingDynLevin(vp[i-3*d], vp[i-d], vp[i+d], vp[i+3*d], w);
        vw[i] = interpolateUsingDynLevin(vw[i-3*d], vw[i-d], vw[i+d], vw[i+3*d], w);

        //  v = ... |   |i-5d|   |   |   |i-3d|   |   |   |i-d|   | i |   |i+d|   |   |   |i+3d|   |   |   |i+5d|   | ...

        if (d>1 && computeSupplementaryAngle(vp[i-d], vp[i], vp[i+d]) > maxAngle)
        {
            // Compute i-2d and i+2d to satisfy precondition when recursing. These
            // are temporary values required to compute i-d/2 and i+d/2, but
            // shouldn't be added as sample (i.e., not added to vi)

            vp[i-2*d] = interpolateUsingDynLevin(vp[i-5*d], vp[i-3*d], vp[i-d], vp[i+d], w);
            vw[i-2*d] = interpolateUsingDynLevin(vw[i-5*d], vw[i-3*d], vw[i-d], vw[i+d], w);

            vp[i+2*d] = interpolateUsingDynLevin(vp[i-d], vp[i+d], vp[i+3*d], vp[i+5*d], w);
            vw[i+2*d] = interpolateUsingDynLevin(vw[i-d], vw[i+d], vw[i+3*d], vw[i+5*d], w);

            //  v = ... |   |i-5d|   |   |   |i-3d|   |i-2d|   |i-d|   | i |   |i+d|   |i+2d|   |i+3d|   |   |   |i+5d|   | ...

            // Recurse depth-first, adding samples in arclength order

            const unsigned int d2 = d / 2;
            recurse_(i-d2, d2);
            sampleIndices.push_back(i);
            recurse_(i+d2, d2);

            //  v = ... |   |i-5d|   |   |   |i-3d|   |i-2d|   |i-d|i-d2| i |i+d2|i+d|   |i+2d|   |i+3d|   |   |   |i+5d|   | ...
        }
        else
        {
            sampleIndices.push_back(i);
        }
    }
};

}

void VCurve::computeSamples_()
{
    // Preconditions:
    //     knots_.size() > 0
    //     end knots are corner knots
    //     consecutive knots have a distance > eps

    const size_t n = knots_.size();
    assert(n>0);
    assert(knots_[0].isCorner);
    assert(knots_[n-1].isCorner);

    // Parameters
    const double eps = 1e-10;

    // Clear samples
    samples_.clear();

    // Allocate the AdaptiveSampler. This shares data and memory accross knots
    AdaptiveSampler sampler(params_.maxNumSubdivision, params_.maxSampleAngle, params_.w);

    // Create all but last sample
    for(unsigned int i=0; i<n-1; ++i)
    {
        // In this loop, we create the samples between knots_[i] and
        // knots_[i+1].
        //
        // For this, we also need to access the two previous knots, and the two
        // following knots (saturating at corner knots). So in total, we need 6
        // knots A, B, C, D, E, F, to compute the samples between C = knots_[i]
        // and D = knots_[i+1].

        // Get knots at i and i+1
        const unsigned int iC = i;
        const unsigned int iD = i+1;
        const VCurveKnot & C = knots_[iC];
        const VCurveKnot & D = knots_[iD];

        // Get knot at "i-1"
        const unsigned int iB = C.isCorner ? iC : iC - 1;
        const VCurveKnot & B = knots_[iB];

        // Get knot at "i-2"
        const unsigned int iA = B.isCorner ? iB : iB - 1;
        const VCurveKnot & A = knots_[iA];

        // Get knot at "i+2"
        const unsigned int iE = D.isCorner ? iD : iD + 1;
        const VCurveKnot & E = knots_[iE];

        // Get knot at "i+3"
        const unsigned int iF = E.isCorner ? iE : iE + 1;
        const VCurveKnot & F = knots_[iF];

        // Adaptive sampling
        // This is where the actual computation happens
        sampler.computeSampling(A, B, C, D, E, F);

        // Remove duplicates and compute samples' arclength
        std::vector<VCurveSample> samples;

        // First sample
        VCurveSample sC; // sample at C
        sC.position = sampler.position(0);
        sC.width    = sampler.width(0);
        if (i == 0)
        {
            sC.arclength = 0.0;
        }
        else
        {
            VCurveSample & s0 = samples_.back();

            const glm::dvec2 dp = sC.position - s0.position;
            const double ds     = glm::length(dp);

            sC.arclength = s0.arclength + ds;
        }
        samples.push_back(sC);

        // Other samples
        for (unsigned int j=0; j<sampler.numSamples(); ++j)
        {
            VCurveSample & s0 = samples.back(); // Note: samples, not samples_.

            const glm::dvec2 & p1 = sampler.position(j);
            const double       w1 = sampler.width(j);

            const glm::dvec2 dp = p1 - s0.position;
            const double ds     = glm::length(dp);

            if (ds > eps) // should be true at least once, since distance(B,C) > eps
            {
                VCurveSample sample;
                sample.position  = p1;
                sample.width     = w1;
                sample.arclength = s0.arclength + ds;
                samples.push_back(sample);
            }
        }

        // Midcondition:
        //   (I)  samples.size() >= 1
        //   (LI) samples.size() >= 2

        if (samples.size() == 1)
        {
            unsigned int j = sampler.numSamples() - 1;

            VCurveSample sample;
            sample.position  = sampler.position(j);
            sample.width     = sampler.width(j);
            sample.arclength = glm::length(sample.position - samples[0].position);
            samples.push_back(sample);
        }

        // Midcondition:
        //   (I) samples.size() >= 2

        assert(samples.size() >= 2);

        // Compute tangents and normals

        // First sample
        if (C.isCorner)
        {
            VCurveSample & s0 = samples[0];
            VCurveSample & s1 = samples[1];

            const glm::dvec2 dp = s1.position  - s0.position;
            const double ds     = s1.arclength - s0.arclength;

            // Compute tangent
            if (ds > eps)
                s0.tangent = dp / ds;
            else
                s0.tangent = glm::dvec2(1.0, 0.0); // should not happen

            // Compute normal
            s0.normal.x = - s0.tangent.y;
            s0.normal.y = + s0.tangent.x;
        }
        else
        {
            VCurveSample & s0 = samples_.back();
            VCurveSample & s1 = samples[0];
            VCurveSample & s2 = samples[1];

            // Compute difference between the sample before and the sample after
            const glm::dvec2 dp = s2.position - s0.position;
            const double ds = glm::length(dp);

            // Compute tangent
            if (ds > eps)
                s1.tangent = dp / ds;
            else
                s1.tangent = glm::dvec2(1.0, 0.0); // very unlikely to happen

            // Compute normal
            s1.normal.x = - s1.tangent.y;
            s1.normal.y = + s1.tangent.x;
        }

        // Other samples except last
        for (unsigned int k=1; k<samples.size()-1; ++k)
        {
            // Get samples before, at, and after i
            VCurveSample & s0 = samples[k-1];
            VCurveSample & s1 = samples[k];
            VCurveSample & s2 = samples[k+1];

            // Compute difference between the sample before and the sample after
            const glm::dvec2 dp = s2.position - s0.position;
            const double ds = glm::length(dp);

            // Compute tangent
            if (ds > eps)
                s1.tangent = dp / ds;
            else
                s1.tangent = glm::dvec2(1.0, 0.0); // very unlikely to happen

            // Compute normal
            s1.normal.x = - s1.tangent.y;
            s1.normal.y = + s1.tangent.x;
        }

        // In case C is a true corner knot (not an end knot), now is the time
        // to add its in-place samples in order to have a nice round join.
        //
        if (C.isCorner && i>0)
        {
            // Midcondition:
            //   samples.size() >= 2
            //   samples_.size() >=1
            //   distance(samples_.back(), samples[0] ) > eps
            //   distance(samples[0],      samples[1] ) > eps

            assert(samples_.size() >= 1);

            const VCurveSample & s0 = samples_.back();
            const VCurveSample & s1 = samples[0];
            const VCurveSample & s2 = samples[1];

            const glm::dvec2 & p0 = s0.position;
            const glm::dvec2 & p1 = s1.position;
            const glm::dvec2 & p2 = s2.position;

            double a1 = std::atan2(p1.y-p0.y, p1.x-p0.x);
            double a2 = std::atan2(p2.y-p1.y, p2.x-p1.x);

            // Compute angle equivalent to a2, closest to a1
            if (a2 > a1 + M_PI)
                a2 -= 2 * M_PI;
            else if (a2 < a1 - M_PI)
                a2 += 2 * M_PI;

            // Compute number of additional samples to create at C.
            // Note: it's ok to have na = 0. That means the "corner" is
            // actually not so much of a corner, and therefore there is no need
            // to add duplicated samples at the corner. samples[0] will still
            // be added no matter what.
            const unsigned int na = std::floor(std::abs(a2-a1) / params_.maxSampleAngle);

            // Create samples
            for (unsigned int k=0; k<na; ++k)
            {
                const double u = (double) k / (double) na;
                const double a = a1 + u*(a2-a1);

                const double cos_a = std::cos(a);
                const double sin_a = std::sin(a);

                VCurveSample sample;
                sample.position  = s1.position;
                sample.width     = s1.width;
                sample.arclength = s1.arclength;
                sample.tangent   = glm::dvec2(cos_a,  sin_a);
                sample.normal    = glm::dvec2(-sin_a, cos_a);
                samples_.push_back(sample);
            }
        }

        // Add samples from B (included) to C (not included)
        // Since samples.size() >= 2, this adds at least
        // one
        for (unsigned int k=0; k<samples.size()-1; ++k)
        {
            samples_.push_back(samples[k]);
        }
    }

    // Create last sample
    VCurveSample lastSample;
    // Position
    lastSample.position = knots_[n-1].position;
    // Width
    lastSample.width = knots_[n-1].width;
    // Arclength + tangent
    if (samples_.size() > 0)
    {
        VCurveSample & s0 = samples_.back();

        const glm::dvec2 dp = lastSample.position  - s0.position;
        const double ds     = glm::length(dp);

        // Arclength
        lastSample.arclength = s0.arclength + ds;

        // Tangent
        if (ds > eps)
            lastSample.tangent = dp / ds;
        else
            lastSample.tangent = glm::dvec2(1.0, 0.0); // should not happen
    }
    else
    {
        lastSample.arclength = 0.0;
        lastSample.tangent = glm::dvec2(1.0, 0.0);
    }
    // Normal
    lastSample.normal.x = - lastSample.tangent.y;
    lastSample.normal.y = + lastSample.tangent.x;
    // Add to samples_
    samples_.push_back(lastSample);

    // Postconditions:
    //     samples_.size() > 0

    assert(samples_.size() > 0);
}

} // end namespace VGeometry
