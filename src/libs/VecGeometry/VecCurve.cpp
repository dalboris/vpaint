// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "VecCurve.h"

#include <glm/geometric.hpp>

VecCurve::VecCurve()
{

}

void VecCurve::clear()
{
    samples_.clear();
}

void VecCurve::addSample(const VecCurveInputSample & inputSample)
{
    // Useful constants
    const glm::vec2 zero(0.0f, 0.0f);
    const glm::vec2 ux(1.0f, 0.0f);
    const glm::vec2 uy(0.0f, 1.0f);
    const int n = size();
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
