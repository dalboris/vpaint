// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "SceneData.h"

#include <glm/geometric.hpp>

size_t SceneData::size() const
{
    return samples.size();
}

void SceneData::clear()
{
    samples.clear();
    arclengths.clear();
    tangents.clear();
}

void SceneData::addSample(const glm::vec2 & centerline)
{
    const glm::vec2 zero(0.0f, 0.0f);
    const int n = size();
    const float eps = 1e-6f;
    const float width = 10.0f;

    if (n == 0)
    {
        SceneDataSample sample;

        sample.centerline1   = centerline;
        sample.normal1       = zero;
        sample.leftBoundary  = centerline;

        sample.centerline2   = centerline;
        sample.normal2       = zero;
        sample.rightBoundary = centerline;

        samples    .push_back(sample);
        arclengths .push_back(0.0f);
        tangents   .push_back(zero);
    }
    else
    {
        // Compute last tangent and arclength
        glm::vec2 tangent = centerline - samples[n-1].centerline1;
        const float length  = glm::length(tangent);

        if (length > eps)
        {
            // Normalize tangent
            tangent /= length;

            // Compute normal
            const glm::vec2 normal(-tangent.y, tangent.x);

            // Compute sample
            SceneDataSample sample;

            sample.centerline1   = centerline;
            sample.normal1       = normal;
            sample.leftBoundary  = centerline + width * normal;

            sample.centerline2   = centerline;
            sample.normal2       = -normal;
            sample.rightBoundary = centerline - width * normal;

            // Add sample
            samples    .push_back(sample);
            arclengths .push_back(arclengths.back() + length);
            tangents   .push_back(tangent);

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
                previousTangent = centerline - samples[n-2].centerline1;
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
            SceneDataSample & previousSample = samples[n-1];
            const glm::vec2 & previousCenterline = previousSample.centerline1;

            previousSample.normal1       = previousNormal;
            previousSample.leftBoundary  = previousCenterline + width * previousNormal;

            previousSample.normal2       = -previousNormal;
            previousSample.rightBoundary = previousCenterline - width * previousNormal;

            tangents[n-1] = previousTangent;
        }
    }
}
