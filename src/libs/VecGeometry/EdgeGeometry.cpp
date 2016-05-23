// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "EdgeGeometry.h"


EdgeGeometry::EdgeGeometry()
{

}

const std::vector<EdgeGeometryGLSample> & EdgeGeometry::samples()
{
    //return samples_;
    return glSamples_;
}

/*
size_t EdgeGeometry::size() const
{
    return samples_.size();
}
*/

void EdgeGeometry::beginFit()
{
    clear();
}

void EdgeGeometry::addFitInputSample(const VecCurveInputSample & inputSample)
{
    //addSample(inputSample);

    curve_.addSample(inputSample);
    computeGLSamples_();
}

void EdgeGeometry::endFit()
{
}

void EdgeGeometry::clear()
{
    //samples_.clear();
    //arclengths_.clear();
    //tangents_.clear();

    curve_.clear();
    glSamples_.clear();

}

void EdgeGeometry::computeGLSamples_()
{
    glSamples_.clear();
    glSamples_.reserve(curve_.numSamples());

    for (VecCurveSample & sample: curve_.samples())
    {
        EdgeGeometryGLSample glSample;

        glSample.left.centerline = sample.position;
        glSample.left.normal     = sample.normal;
        glSample.left.position   = sample.position + sample.width * sample.normal;

        glSample.right.centerline = sample.position;
        glSample.right.normal     = - sample.normal;
        glSample.right.position   = sample.position - sample.width * sample.normal;

        glSamples_.push_back(glSample);
    }
}

/*
void EdgeGeometry::addSample(const VecCurveInputSample & inputSample)
{
    const glm::vec2 zero(0.0f, 0.0f);
    const int n = size();
    const float eps = 1e-6f;

    const glm::vec2 & centerline = inputSample.position;
    const float    & width       = inputSample.width;

    if (n == 0)
    {
        EdgeGeometryGLSample sample;

        sample.left.centerline = centerline;
        sample.left.normal     = zero;
        sample.left.position   = centerline;

        sample.right.centerline = centerline;
        sample.right.normal     = zero;
        sample.right.position   = centerline;

        samples_    .push_back(sample);
        arclengths_ .push_back(0.0f);
        tangents_   .push_back(zero);
    }
    else
    {
        // Compute last tangent and arclength
        glm::vec2 tangent = centerline - samples_[n-1].left.centerline;
        const float length  = glm::length(tangent);

        if (length > eps)
        {
            // Normalize tangent
            tangent /= length;

            // Compute normal
            const glm::vec2 normal(-tangent.y, tangent.x);

            // Compute sample
            EdgeGeometryGLSample sample;

            sample.left.centerline = centerline;
            sample.left.normal     = normal;
            sample.left.position   = centerline + width * normal;

            sample.right.centerline = centerline;
            sample.right.normal     = -normal;
            sample.right.position   = centerline - width * normal;

            // Add sample
            samples_    .push_back(sample);
            arclengths_ .push_back(arclengths_[n-1] + length);
            tangents_   .push_back(tangent);

            // Compute updated tangent of previous sample.
            // Notes: n = size() - 1.
            //        P[n]   is the last element
            //        P[n-1] is the element to update
            glm::vec2 previousTangent;
            if (n == 1)
            {
                // Size is now 2. We use P[1] - P[0] as tangent for P[0]
                previousTangent = tangent;
            }
            else
            {
                // Size is now 3 or more. We use P[n] - P[n-2] as tangent for P[n-1]
                previousTangent = centerline - samples_[n-2].left.centerline;
                float previousLength  = glm::length(previousTangent);
                if (previousLength > eps)
                {
                    // Normalize tangent
                    previousTangent /= previousLength;
                }
                else
                {
                    // Heuristic for degenerate case
                    // XXX could/should we be smarter?
                    previousTangent = normal;
                }
            }

            // Compute updated normal of previous sample
            const glm::vec2 previousNormal(-previousTangent.y, previousTangent.x);

            // Update previous sample
            EdgeGeometryGLSample & previousSample = samples_[n-1];
            const glm::vec2 & previousCenterline = previousSample.left.centerline;

            previousSample.left.normal   = previousNormal;
            previousSample.left.position = previousCenterline + width * previousNormal;

            previousSample.right.normal   = -previousNormal;
            previousSample.right.position = previousCenterline - width * previousNormal;

            tangents_[n-1] = previousTangent;
        }
    }
}
*/
