// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "Scene.h"

#include <glm/geometric.hpp>

Scene::Scene()
{
    data_.layers.push_back(DataObjectPtr<Layer>());
}

Layer * Scene::activeLayer() const
{
    return data().layers[0].get();
}

const std::vector<SceneDataSample> & Scene::samples()
{
    return data_.samples;
}

size_t Scene::size() const
{
    return data_.samples.size();
}

void Scene::beginStroke(const glm::vec2 & centerline)
{
    clear();
    addSample(centerline);
    emit changed();
}

void Scene::continueStroke(const glm::vec2 & centerline)
{
    addSample(centerline);
    emit changed();
}

void Scene::endStroke()
{
}

void Scene::clear()
{
    data_.samples.clear();
    data_.arclengths.clear();
    data_.tangents.clear();
}

void Scene::addSample(const glm::vec2 & centerline)
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

        data_.samples    .push_back(sample);
        data_.arclengths .push_back(0.0f);
        data_.tangents   .push_back(zero);
    }
    else
    {
        // Compute last tangent and arclength
        glm::vec2 tangent = centerline - data_.samples[n-1].centerline1;
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
            data_.samples    .push_back(sample);
            data_.arclengths .push_back(data_.arclengths[n-1] + length);
            data_.tangents   .push_back(tangent);

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
                previousTangent = centerline - data_.samples[n-2].centerline1;
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
            SceneDataSample & previousSample = data_.samples[n-1];
            const glm::vec2 & previousCenterline = previousSample.centerline1;

            previousSample.normal1       = previousNormal;
            previousSample.leftBoundary  = previousCenterline + width * previousNormal;

            previousSample.normal2       = -previousNormal;
            previousSample.rightBoundary = previousCenterline - width * previousNormal;

            data_.tangents[n-1] = previousTangent;
        }
    }
}
