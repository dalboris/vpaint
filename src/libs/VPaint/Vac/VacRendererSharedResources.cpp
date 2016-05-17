// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "Vac/VacRendererSharedResources.h"

#include "Vac/Vac.h"

#include "OpenVac/Topology/KeyEdge.h"

VacRendererSharedResources::VacRendererSharedResources(Vac * vac) :
    vac_(vac)
{
    // XXX TODO: listen to notifications more specific than "changed".
    connect(vac_, &Vac::changed, this, &VacRendererSharedResources::setDirty);
}

VacRendererSharedResources::~VacRendererSharedResources()
{
}

Vac * VacRendererSharedResources::vac() const
{
    return vac_;
}

void VacRendererSharedResources::setDirty()
{
    isDirty_ = true;
}

void VacRendererSharedResources::initialize(OpenGLFunctions * /*f*/)
{
    if (refCount_ == 0)
    {
        // Initialize shader program
        shaderProgram_.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/OpenGL/Shaders/Helloworld.v.glsl");
        shaderProgram_.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/OpenGL/Shaders/Helloworld.f.glsl");
        shaderProgram_.link();

        // Get shader locations
        shaderProgram_.bind();
        vertexLoc_     = shaderProgram_.attributeLocation("vertex");
        projMatrixLoc_ = shaderProgram_.uniformLocation("projMatrix");
        viewMatrixLoc_ = shaderProgram_.uniformLocation("viewMatrix");
        shaderProgram_.release();

        // Create VBO
        vbo_.create();
    }

    ++refCount_;
}

void VacRendererSharedResources::update(OpenGLFunctions * /*f*/)
{
    if (isDirty_)
    {
        // Get all edges
        std::vector<OpenVac::KeyEdgeHandle> edges;
        std::vector<OpenVac::CellHandle> cells =  vac()->data().cells();
        for (const OpenVac::CellHandle & cell: cells)
        {
            OpenVac::KeyEdgeHandle edge = cell;
            if (edge)
                edges.push_back(edge);
        }

        // Get total num samples
        int numSamples = 0;
        for (const OpenVac::KeyEdgeHandle & edge: edges)
        {
            const auto & samples = edge->geometry().samples();
            numSamples += samples.size();
        }

        // Allocate VBO (i.e., allocate memory in GPU)
        vbo_.bind();
        vbo_.allocate(numSamples * sizeof(EdgeGeometryGpuSample));
        vbo_.release();

        // Write VBO (i.e., copy data from CPU to GPU)
        int offset = 0;
        vbo_.bind();
        for (const OpenVac::KeyEdgeHandle & edge: edges)
        {
            const auto & samples = edge->geometry().samples();
            int count = samples.size() * sizeof(EdgeGeometryGpuSample);

            vbo_.write(offset, samples.data(), count);

            offset += count;
        }
        vbo_.release();
    }

    isDirty_ = false;
}

void VacRendererSharedResources::cleanup(OpenGLFunctions * /*f*/)
{
    --refCount_;

    if (refCount_ == 0)
    {
        vbo_.destroy();
    }
}
