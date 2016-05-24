// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "VecCurve.h"

#include <glm/geometric.hpp>

VecCurve::VecCurve() :
    numFitSamples_(10),
    samplingRate_(0.01f) // 10ms
{

}

void VecCurve::clear()
{
    samples_.clear();

    inputSamples_.clear();
    inputUniformSampling_.clear();

}

bool VecCurve::addSampleIfNotTooCloseFromPrevious_(const VecCurveInputSample & inputSample)
{
    const int n = inputSamples_.size();
    lastSample_ = inputSample; // we keep it for endFit(): a previously discarded sample
                               // can be added at the very end. XXX TODO.
    if (n == 0)
    {
        inputSamples_.push_back(inputSample);
        return true;
    }
    else
    {
        // Get previous sample
        const VecCurveInputSample & prevInputSample = inputSamples_[n-1];

        // Get distance between samples
        const float distance =  glm::length(inputSample.position - prevInputSample.position);

        // Get distance in time between samples
        const float dt =  inputSample.time - prevInputSample.time;

        // append if not too close
        if (distance > 3*inputSample.resolution && dt > samplingRate_)
        {
            inputSamples_.push_back(inputSample);
            return true;
        }
        else
        {
            return false;
        }
    }
}

void VecCurve::computeInputUniformSampling_()
{
    const unsigned int n = inputSamples_.size();
    inputUniformSampling_.clear();

    // Note: at this stage, attributes other than position and width are ignored

    if (n == 0)
    {
        // Nothing to do
    }
    else if (n == 1)
    {
        VecCurveSample sample;
        sample.position = inputSamples_[0].position;
        sample.width = inputSamples_[0].width;
        inputUniformSampling_.push_back(sample);
    }
    else // n >= 2
    {
        // Add first sample. We do not include this is the
        // loop below to guarantee this is added even when tMax <=0
        //
        VecCurveSample firstSample;
        firstSample.position = inputSamples_[0].position;
        firstSample.width = inputSamples_[0].width;
        inputUniformSampling_.push_back(firstSample);

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
        float tMax = inputSamples_[n-1].time - 0.5*samplingRate_;

        // Index of inputSamples_ such that:
        //     inputSamples_[i].time <= t < inputSamples_[i+1].time
        //
        // This value is updated inside the loop. It is guaranteed that
        // i <= 0 <= n-2 (i.e., both i and i+1 are always valid indexes).
        //
        unsigned int i = 0;

        // Loop over all samples to compute (not including the first and last)
        //
        for (float t = samplingRate_; t < tMax; t += samplingRate_)
        {
            // Update i
            while (i+2 < n && t >= inputSamples_[i+1].time)
            {
                ++i;
            }

            // Get samples from which to compute Cubic Hermite
            VecCurveInputSample & s0 = inputSamples_[i>0 ? i-1 : 0];
            VecCurveInputSample & s1 = inputSamples_[i];
            VecCurveInputSample & s2 = inputSamples_[i+1];
            VecCurveInputSample & s3 = inputSamples_[i<n-1 ? i+1 : n-1];

            // Get times
            const float & t0 = s0.time;
            const float & t1 = s1.time;
            const float & t2 = s2.time;
            const float & t3 = s3.time;

            // Get positions
            const glm::vec2 & p0 = s0.position;
            const glm::vec2 & p1 = s1.position;
            const glm::vec2 & p2 = s2.position;
            const glm::vec2 & p3 = s3.position;

            // Get widths
            const float & w0 = s0.width;
            const float & w1 = s1.width;
            const float & w2 = s2.width;
            const float & w3 = s3.width;

            // Compute delta of times. Note: we know they are > 0 because
            // dt > samplingRate_ for each consecutive samples
            const float dt  = t2 - t1;
            const float dt1 = t2 - t0;
            const float dt2 = t3 - t1;

            // Catmull-Rom Heuristic for desired derivatives of position
            const glm::vec2 dp1 = (p2 - p0) / dt1;
            const glm::vec2 dp2 = (p3 - p1) / dt2;

            // Catmull-Rom Heuristic for desired derivatives of widths
            const float dw1 = (w2 - w0) / dt1;
            const float dw2 = (w3 - w1) / dt2;

            // Cubic Hermite coefficients, to interpolate the desired
            // data points with the desired derivatives at t.
            //
            // Note that the coefficients dc1 and dc2 are pre-multiplied
            // by dt to scale the derivatives. This accounts for
            // the fact that t is in [t1, t2], while u is in [0, 1]
            const float u = (t - t1) / dt;
            const float u2 = u*u;
            const float u3 = u2*u;
            const float c1 = 2*u3 - 3*u2 + 1;
            const float c2 = - 2*u3 + 3*u2;
            const float dc1 = dt * (u3 - 2*u2 + u);
            const float dc2 = dt * (u3 - u2);

            // Compute interpolated sample
            VecCurveSample sample;
            sample.position = c1*p1 + c2*p2 + dc1*dp1 + dc2*dp2;
            sample.width    = c1*w1 + c2*w2 + dc1*dw1 + dc2*dw2;
            inputUniformSampling_.push_back(sample);
        }

        // Add last sample
        VecCurveSample lastSample;
        lastSample.position = inputSamples_[n-1].position;
        lastSample.width = inputSamples_[n-1].width;
        inputUniformSampling_.push_back(lastSample);
    }
}

void VecCurve::addSample(const VecCurveInputSample & inputSample)
{
    // Insert sample
    bool inserted = addSampleIfNotTooCloseFromPrevious_(inputSample);

    // Compute uniform sampling
    if (!inserted)
        inputSamples_.push_back(lastSample_);

    computeInputUniformSampling_();

    if (!inserted)
        inputSamples_.pop_back();

    // XXX TEMP
    samples_ = inputUniformSampling_;
    const int n = samples_.size();
    // compute tangents and normals
    for (int i=0; i<n; ++i)
    {
        // Get samples before, at, and after i
        VecCurveSample & s0 = samples_[i>0 ? i-1 : 0];
        VecCurveSample & s1 = samples_[i];
        VecCurveSample & s2 = samples_[i<n-1 ? i+1 : n-1];

        // Compute difference between the sample before and the sample after
        const glm::vec2 dp = s2.position - s0.position;
        const float ds = glm::length(dp);

        // Compute tangent
        if (ds > 1e-6)
            s1.tangent = dp / ds;
        else
            s1.tangent = glm::vec2(1.0f, 0.0f);

        // Compute normal
        s1.normal.x = - s1.tangent.y;
        s1.normal.y = + s1.tangent.x;
    }

    // End function here (not to execute previous code)
    // XXX this is temporary
    return;



    // ----------- old code below --------------

    // Useful constants
    const glm::vec2 zero(0.0f, 0.0f);
    const glm::vec2 ux(1.0f, 0.0f);
    const glm::vec2 uy(0.0f, 1.0f);
    //const int n = samples_.size();
    const float eps = 0.1f * inputSample.resolution;

    // Switch depending on current number of samples
    if (n == 0)
    {
        VecCurveSample sample;
        sample.position = inputSample.position;
        sample.width = inputSample.width;
        sample.tangent = ux;
        sample.normal = uy;
        sample.arclength = 0.0f;
        sample.angle = 0.0f;
        push_back(sample);
    }
    else
    {
        // Get previous sample
        VecCurveSample & prevSample = samples_[n-1];

        // Compute difference with previous sample
        const glm::vec2 dp = inputSample.position - prevSample.position;
        const float ds = glm::length(dp);

        // Discard sample if too close
        if (ds > eps)
        {
            // Sample to compute
            VecCurveSample sample;

            // Set position
            sample.position = inputSample.position;

            // Set width
            sample.width = inputSample.width;

            // Compute tangent
            sample.tangent = dp / ds;

            // Compute normal
            sample.normal.x = - sample.tangent.y;
            sample.normal.y = + sample.tangent.x;

            // Compute arclength
            sample.arclength = prevSample.arclength + ds;

            // Compute angle XXX TODO
            sample.angle = 0.0f;

            // Update tangent and normal of previous sample
            if (n == 1)
            {
                prevSample.tangent = sample.tangent;
                prevSample.normal  = sample.normal;
            }
            else // size >= 2
            {
                // Get sample before previous sample
                VecCurveSample & prevSample2 = samples_[n-2];

                // Compute difference between this sample and prevSample2
                const glm::vec2 dp2 = inputSample.position - prevSample2.position;
                const float ds2 = glm::length(dp2);

                // Update tangent of previous sample
                if (ds2 > eps)
                {
                    prevSample.tangent = dp2 / ds2;
                }
                else
                {
                    prevSample.tangent = ux;
                }

                // Update normal of previous sample
                prevSample.normal.x = - prevSample.tangent.y;
                prevSample.normal.y = + prevSample.tangent.x;
            }

            // Insert sample
            push_back(sample);
        }
    }
}

size_t VecCurve::numSamples() const
{
    return samples_.size();
}

const VecCurveSample & VecCurve::sample(unsigned int i) const
{
    return samples_.at(i);
}


VecCurveSample VecCurve::sample(double s) const
{
    return VecCurveSample();
}


double VecCurve::length() const
{
    if (size() == 0)
    {
        return 0.0;
    }
    else
    {
        return samples_.back().arclength;
    }
}

size_t VecCurve::size() const
{
    return samples_.size();
}

void VecCurve::push_back(const VecCurveSample & sample)
{
    samples_.push_back(sample);
}

void VecCurve::push_back(VecCurveSample && sample)
{
    samples_.push_back(std::move(sample));
}

VecCurveSample & VecCurve::operator[](int i)
{
    return samples_[i];
}

std::vector<VecCurveSample> & VecCurve::samples()
{
    return samples_;
}
