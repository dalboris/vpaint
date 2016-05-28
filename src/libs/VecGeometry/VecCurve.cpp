// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "VecCurve.h"
#include "VecFitter.h"

#include <glm/geometric.hpp>

VecCurve::VecCurve() :
    samplingRate_(0.005f), // 5ms
    numFitSamples_(40)
{

}

void VecCurve::clear()
{
    samples_.clear();

    inputSamples_.clear();

    inputUniformSamplingPosition_.clear();
    inputUniformSamplingWidth_.clear();

    fitters_.clear();
    smoothedUniformSamplingPosition_.clear();
    smoothedUniformSamplingWidth_.clear();
}

bool VecCurve::addSampleIfNotTooCloseFromPrevious_(const VecCurveInputSample & inputSample)
{
    const int n = inputSamples_.size();

    // Remeber last sample
    lastSample_ = inputSample;

    // Set time based on index, i.e.: constant time between two samples.
    // Indeed, it turns out that this works better. Using actual time causes
    // issues when the user stop drawing. How to know whether it's a lag, or an
    // intentional slow down?
    lastSample_.time = 4*samplingRate_*n;

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
        // Not that here we still use inputSample.time and not lastSample_.time.
        const float dt =  inputSample.time - prevInputSample.time;

        // append if not too close
        if (distance > 0.1*inputSample.resolution) // && dt > 0.5*samplingRate_)
        {
            inputSamples_.push_back(lastSample_);
            return true;
        }
        else
        {
            return false;
        }
    }
}

void computeNoiseFitters_()
{

}

void VecCurve::computeInputUniformSampling_()
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
            const glm::vec2 position = c1*p1 + c2*p2 + dc1*dp1 + dc2*dp2;
            const float     width    = c1*w1 + c2*w2 + dc1*dw1 + dc2*dw2;

            // Insert sample
            inputUniformSamplingPosition_.push_back(position);
            inputUniformSamplingWidth_.push_back(width);
        }

        // Add last sample
        inputUniformSamplingPosition_.push_back(inputSamples_[n-1].position);
        inputUniformSamplingWidth_.push_back(inputSamples_[n-1].width);
    }
}

#include <iostream>

void VecCurve::smoothUniformSampling_()
{
    const size_t n = inputUniformSamplingPosition_.size();
    const glm::vec2 zero(0.0f, 0.0f);

    // Smooth positions via fitting
    smoothedUniformSamplingPosition_.resize(n);

    if (n <= numFitSamples_)
    {
        // Only use one fitter
        VecFitter fitter(inputUniformSamplingPosition_);

        // Sample along the fitter
        for (unsigned int i=0; i<n; ++i)
        {
            float u = (float) i / (float) (n-1);
            smoothedUniformSamplingPosition_[i] = fitter(u);
        }
    }
    else // n >= numFitSamples_ + 1
    {
        // Only two ot more fitters
        const size_t numFittersBefore = fitters_.size();
        const size_t numFitters = n - numFitSamples_ + 1;
        fitters_.resize(numFitters);

        // Index of the last existing fitter (or zero if no fitter yet)
        const unsigned int iLastFitter = numFittersBefore > 0 ? numFittersBefore-1 : 0;

        // Re-compute last existing fitter, and compute all new fitters
        for (unsigned int i=iLastFitter; i<numFitters; ++i)
        {
            std::vector<glm::vec2> samples(numFitSamples_);
            for (unsigned int j=0; j<numFitSamples_; ++j)
            {
                samples[j] = inputUniformSamplingPosition_[i+j];
            }
            fitters_[i] = VecFitter(samples);
        }

        //std::cout << "n=" << n << " numFitSamples_=" << numFitSamples_ << " numFitters=" << numFitters << std::endl;


        // Precompute u. Note: we could precompute u2 and u3 too, but
        // this only save one multiplication. Two memory accesses are
        // likely slower than one multiplication
        std::vector<float> u(numFitSamples_);
        for (unsigned int j=0; j<numFitSamples_; ++j)
        {
            u[j] = (float) j / (float) (numFitSamples_-1);
        }


        // First sample
        smoothedUniformSamplingPosition_[0] = inputUniformSamplingPosition_[0];

        // Re-compute existing but affected samples, and compute all new samples
        for (unsigned int i=0 /*iLastFitter*/; i<n-1; ++i)
        {
            glm::vec2 pos = zero;
            float sumW = 0.0f;
            /*
            for (unsigned int j=0; j<numFitSamples_; ++j)
            {
            */
            unsigned int j = i % numFitSamples_;

                //const int k = (int)i - (int)j;
                 int k = (int)i - (int)j;
                // XXX TEMP
                if (k >= (int)numFitters)
                {
                    k = (int)numFitters -1;
                    j = i - k;
                }

                //std::cout << "i=" << i << " j=" << j << " k=" << k << std::endl;
                if (0 <= k && k < (int)numFitters)
                {
                    const VecFitter & fitter = fitters_[k];
                    const float uj = fitter.uis()[j];

                    const glm::vec2 posj = fitter(uj);
                    //const float     wj   = w_(u[j]);
                    const float     wj   = 1.0;

                    pos  += wj * posj;
                    sumW += wj;
                }
                else
                {
                    std::cout << "weird" << std::endl;
                }
            //}
            smoothedUniformSamplingPosition_[i] = (1.0f / sumW) * pos;
        }

        // Last sample
        smoothedUniformSamplingPosition_[n-1] = inputUniformSamplingPosition_[n-1];
    }

    // Do not smooth width for now
    smoothedUniformSamplingWidth_ = inputUniformSamplingWidth_;
}

void VecCurve::computeFinalSamples_()
{
    const size_t n = regularizedPositions_.size();
    samples_.resize(n);

    // Set position and width
    for (unsigned int i=0; i<n; ++i)
    {
        VecCurveSample & sample = samples_[i];

        sample.position = regularizedPositions_[i];
        sample.width    = regularizedWidth_[i];
    }

    // Compute tangents and normals
    for (unsigned int i=0; i<n; ++i)
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
}

void VecCurve::computeNoiseFitters_()
{
    const size_t n = inputSamples_.size();
    assert(n > 0);

    const size_t numSamplesToFit = std::min((size_t) 6, n);
    const size_t numSubSamples = 1;
    const size_t numSubSamplesToFit = numSubSamples * (numSamplesToFit-1) + 1;

    const size_t numNoiseFitters = n - numSamplesToFit + 1;
    noiseFitters_.resize(numNoiseFitters);

    for (unsigned int i=0; i<numNoiseFitters; ++i)
    {
        std::vector<glm::vec2> subSamplesToFit(numSubSamplesToFit);

        // Full-samples
        const bool applyConvolution = false;
        for (unsigned int j=0; j<numSamplesToFit; ++j)
        {
            const unsigned int k = i+j;
            if (applyConvolution && k > 0 && k < n-1)
            {
                subSamplesToFit[j*numSubSamples] =
                        0.25f * inputSamples_[k-1].position +
                        0.50f * inputSamples_[k].position +
                        0.25f * inputSamples_[k+1].position;
            }
            else
            {
                subSamplesToFit[j*numSubSamples] = inputSamples_[k].position;
            }
        }

        // Sub-samples
        for (unsigned int j=0; j<numSamplesToFit-1; ++j)
        {
            for (unsigned int k=1; k<numSubSamples; ++k)
            {
                const float u = (float) k / (float) (numSubSamples - 1);
                subSamplesToFit[j*numSubSamples+k] = (1-u) * subSamplesToFit[j*numSubSamples] + u * subSamplesToFit[(j+1)*numSubSamples];
            }
        }

        // Compute fit
        noiseFitters_[i] = VecFitter(subSamplesToFit);
    }
}

void VecCurve::averageNoiseFitters_()
{
    const size_t n = inputSamples_.size();
    const size_t numNoiseFitters = noiseFitters_.size();
    const size_t numSamplesToFit = n - numNoiseFitters + 1;

    // Positions
    regularizedPositions_.resize(n);
    regularizedPositions_[0] = inputSamples_[0].position;
    for (unsigned int i=1; i<n-1; ++i)          // i = global index of sample
    {
        glm::dvec2 pos(0.0, 0.0);
        double sumW = 0.0;

        for (unsigned int j=1; j<numSamplesToFit-1; ++j) // j = index of sample w.r.t fitter
            // loop range equivalent to do j in [0, numSamplesToFit)
            // since w_(uj) = 0.0 for j = 0 and j = numSamplesToFit-1
        {
            const int k = (int)i - (int)j; // k = index of fitter whose j-th sample is samples[i]
            if (0 <= k && k < (int)numNoiseFitters)
            {
                const VecFitter & fitter = noiseFitters_[k];
                const double uj1 = (double) j / (double) (numSamplesToFit-1);
                const double uj2 = fitter.uis()[j];

                const glm::dvec2 posj = fitter(uj1);
                const double     wj   = w_(uj1);

                pos  += wj * posj;
                sumW += wj;
            }
        }
        regularizedPositions_[i] = (1/sumW) * pos;
    }
    regularizedPositions_[n-1] = inputSamples_[n-1].position;

    // Widths
    regularizedWidth_.resize(n);
    for (unsigned int i=0; i<n; ++i)
    {
        regularizedWidth_[i] = inputSamples_[i].width;
    }
}

void VecCurve::computeConvolution_()
{
    const size_t n = inputSamples_.size();
    assert(n > 0);

    convolutedSamples_.resize(n);
    convolutedSamples_[0] = inputSamples_[0].position;
    for (unsigned int i=1; i<n-1; ++i)
    {
        convolutedSamples_[i] =
                0.25f * inputSamples_[i-1].position +
                0.50f * inputSamples_[i].position +
                0.25f * inputSamples_[i+1].position;
    }
    convolutedSamples_[n-1] = inputSamples_[n-1].position;
}

void VecCurve::addSample(const VecCurveInputSample & inputSample)
{
    // Insert sample (or not) in inputSamples_
    bool inserted = addSampleIfNotTooCloseFromPrevious_(inputSample);

    // Add last sample anyway for now, even though it may be eventually
    // discarded if the user keeps drawing
    if (!inserted)
        inputSamples_.push_back(lastSample_);

    // Process the curve
    /*
    computeInputUniformSampling_();
    smoothUniformSampling_();
    computeFinalSamples_();
    */

    computeNoiseFitters_();
    averageNoiseFitters_();
    computeConvolution_();
    computeFinalSamples_();

    // Remove last sample if eventually discarded
    if (!inserted)
        inputSamples_.pop_back();
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
