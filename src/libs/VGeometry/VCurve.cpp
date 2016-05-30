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

size_t VCurve::numSamples() const
{
    return samples_.size();
}

const VCurveSample & VCurve::sample(unsigned int i) const
{
    return samples_.at(i);
}

double VCurve::length() const
{
    return numSamples() == 0 ? 0.0 : samples_.back().arclength;
}

void VCurve::appendInputSample_(const VCurveInputSample & inputSample)
{
    const int n = inputSamples_.size();

    if (n >= 2)
    {
        // Get previous sample
        const VCurveInputSample & prevInputSample = inputSamples_[n-1];

        // Get the sample before that
        const VCurveInputSample & prev2InputSample = inputSamples_[n-2];

        // Get distance between samples
        const double distance =  glm::length(prevInputSample.position - prev2InputSample.position);

        // Discard last sample if too close
        if (distance < 0.1*inputSample.resolution)
        {
            inputSamples_.pop_back();
        }
    }

    // Append this sample
    inputSamples_.push_back(inputSample);
}

void VCurve::computeRegPositions_()
{
    computeRegFits_();
    averageRegFits_();
}

void VCurve::computeRegWidths_()
{
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
}

void VCurve::computeRegFits_()
{
    const size_t n = inputSamples_.size();
    assert(n > 0);

    const size_t numSamplesPerFit = std::min((size_t) 5, n);
    const size_t numFits          = n - numSamplesPerFit + 1;

    regFits_.resize(numFits);
    for (unsigned int i=0; i<numFits; ++i)
    {
        std::vector<glm::dvec2> samplesToFit(numSamplesPerFit);
        for (unsigned int j=0; j<numSamplesPerFit; ++j)
        {
            samplesToFit[j] = inputSamples_[i+j].position;
        }
        regFits_[i] = fitQuadratic(samplesToFit);
    }
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
    const size_t n                     = inputSamples_.size();
    const size_t numCubicFits          = regFits_.size();
    const size_t numSamplesPerCubicFit = n - numCubicFits + 1;

    regPositions_.resize(n);
    regPositions_[0] = inputSamples_[0].position;
    for (unsigned int i=1; i<n-1; ++i)          // i = global index of sample
    {
        glm::dvec2 pos(0.0, 0.0);
        double sumW = 0.0;

        for (unsigned int j=1; j<numSamplesPerCubicFit-1; ++j) // j = index of sample w.r.t fitter
            // loop range equivalent to do j in [0, numSamplesToFit)
            // since w_(uj) = 0.0 for j = 0 and j = numSamplesToFit-1
        {
            const int k = (int)i - (int)j; // k = index of fitter whose j-th sample is samples[i]
            if (0 <= k && k < (int)numCubicFits)
            {
                const CubicCurve & cubicFit = regFits_[k];
                const double uj = (double) j / (double) (numSamplesPerCubicFit-1);

                const glm::dvec2 posj = cubicFit.pos(uj);
                const double     wj   = w_(uj);

                pos  += wj * posj;
                sumW += wj;
            }
        }
        regPositions_[i] = (1/sumW) * pos;
    }
    regPositions_[n-1] = inputSamples_[n-1].position;
}

void VCurve::computeSamples_()
{
    const size_t n = regPositions_.size();
    samples_.resize(n);

    // Set position and width
    for (unsigned int i=0; i<n; ++i)
    {
        VCurveSample & sample = samples_[i];

        sample.position = regPositions_[i];
        sample.width    = regWidths_[i];
    }

    // Compute tangents and normals
    for (unsigned int i=0; i<n; ++i)
    {
        // Get samples before, at, and after i
        VCurveSample & s0 = samples_[i>0 ? i-1 : 0];
        VCurveSample & s1 = samples_[i];
        VCurveSample & s2 = samples_[i<n-1 ? i+1 : n-1];

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


/* XXX Old code kept in comment for now, since I'll use the catmull-rom code soon
 *
void VCurve::computeInputUniformSampling_()
{
    const unsigned int n = inputSamples_.size();
    inputUniformSamplingPosition_.clear();
    inputUniformSamplingWidth_.clear();

    // Note: at this stage, attributes other than position and width are ignored

    if (n == 0)
    {
        // Nothing to do
    }
    else if (n == 1)
    {
        inputUniformSamplingPosition_.push_back(inputSamples_[0].position);
        inputUniformSamplingWidth_.push_back(inputSamples_[0].width);
    }
    else // n >= 2
    {
        // Add first sample. We do not include this is the
        // loop below to guarantee this is added even when tMax <=0
        //
        inputUniformSamplingPosition_.push_back(inputSamples_[0].position);
        inputUniformSamplingWidth_.push_back(inputSamples_[0].width);

        // Find time when to quit the loop.
        //
        // Why this isn't simply inputSamples_[n-1].time ?
        //
        // Remember: Consecutives samples are exactly separated by
        // 'samplingRate_', but with one exeption: the last two samples.
        // Indeed, we always add the last sample regardless of its
        // 'time' attrubute. In order to prevent that the last two
        // samples are too close in time, we stop the loop before,
        // to ensure that the last two samples are always separated
        // by at least 0.5*samplingRate_.
        //
        // Note:
        // If:   inputSamples_[n-1].time <= 1.5 * samplingRate_
        // Then:           samplingRate_ >= inputSamples_[n-1].time - 0.5 * samplingRate_
        // Then:           samplingRate_ >= tMax
        // Then: the loop is never executed.
        //
        // In this case, inputUniformSampling_ will be made of two samples only,
        // and they may be closer than 0.5*samplingRate_.
        //
        double tMax = inputSamples_[n-1].time - 0.5*samplingRate_;

        // Index of inputSamples_ such that:
        //     inputSamples_[i].time <= t < inputSamples_[i+1].time
        //
        // This value is updated inside the loop. It is guaranteed that
        // i <= 0 <= n-2 (i.e., both i and i+1 are always valid indexes).
        //
        unsigned int i = 0;

        // Loop over all samples to compute (not including the first and last)
        //
        for (double t = samplingRate_; t < tMax; t += samplingRate_)
        {
            // Update i
            while (i+2 < n && t >= inputSamples_[i+1].time)
            {
                ++i;
            }

            // Get samples from which to compute Cubic Hermite
            VCurveInputSample & s0 = inputSamples_[i>0 ? i-1 : 0];
            VCurveInputSample & s1 = inputSamples_[i];
            VCurveInputSample & s2 = inputSamples_[i+1];
            VCurveInputSample & s3 = inputSamples_[i<n-1 ? i+1 : n-1];

            // Get times
            const double & t0 = s0.time;
            const double & t1 = s1.time;
            const double & t2 = s2.time;
            const double & t3 = s3.time;

            // Get positions
            const glm::dvec2 & p0 = s0.position;
            const glm::dvec2 & p1 = s1.position;
            const glm::dvec2 & p2 = s2.position;
            const glm::dvec2 & p3 = s3.position;

            // Get widths
            const double & w0 = s0.width;
            const double & w1 = s1.width;
            const double & w2 = s2.width;
            const double & w3 = s3.width;

            // Compute delta of times. Note: we know they are > 0 because
            // dt > samplingRate_ for each consecutive samples
            const double dt  = t2 - t1;
            const double dt1 = t2 - t0;
            const double dt2 = t3 - t1;

            // Catmull-Rom Heuristic for desired derivatives of position
            const glm::dvec2 dp1 = (p2 - p0) / dt1;
            const glm::dvec2 dp2 = (p3 - p1) / dt2;

            // Catmull-Rom Heuristic for desired derivatives of widths
            const double dw1 = (w2 - w0) / dt1;
            const double dw2 = (w3 - w1) / dt2;

            // Cubic Hermite coefficients, to interpolate the desired
            // data points with the desired derivatives at t.
            //
            // Note that the coefficients dc1 and dc2 are pre-multiplied
            // by dt to scale the derivatives. This accounts for
            // the fact that t is in [t1, t2], while u is in [0, 1]
            const double u = (t - t1) / dt;
            const double u2 = u*u;
            const double u3 = u2*u;
            const double c1 = 2*u3 - 3*u2 + 1;
            const double c2 = - 2*u3 + 3*u2;
            const double dc1 = dt * (u3 - 2*u2 + u);
            const double dc2 = dt * (u3 - u2);

            // Compute interpolated sample
            const glm::dvec2 position = c1*p1 + c2*p2 + dc1*dp1 + dc2*dp2;
            const double     width    = c1*w1 + c2*w2 + dc1*dw1 + dc2*dw2;

            // Insert sample
            inputUniformSamplingPosition_.push_back(position);
            inputUniformSamplingWidth_.push_back(width);
        }

        // Add last sample
        inputUniformSamplingPosition_.push_back(inputSamples_[n-1].position);
        inputUniformSamplingWidth_.push_back(inputSamples_[n-1].width);
    }
}
*/
