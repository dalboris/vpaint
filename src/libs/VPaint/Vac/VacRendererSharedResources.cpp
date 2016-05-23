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
    connect(vac, &Vac::topologyChanged, this, &VacRendererSharedResources::onTopologyChanged);
    connect(vac, &Vac::geometryChanged, this, &VacRendererSharedResources::onGeometryChanged);
}

VacRendererSharedResources::~VacRendererSharedResources()
{
}


Vac * VacRendererSharedResources::vac() const
{
    return vac_;
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
    }

    ++refCount_;
}


void VacRendererSharedResources::update(OpenGLFunctions * /*f*/)
{
    // Loop over all destroyed key edges, and destroy corresponding VBO
    //
    for (OpenVac::CellIdType idtype: topologyEditInfo_.destroyed())
    {
        if (idtype.type == OpenVac::CellType::KeyEdge)
        {
            destroyVBO_(idtype.id);
        }
    }

    // Loop over all created key edges, and create one VBO for each
    //
    for (OpenVac::CellIdType idtype: topologyEditInfo_.created())
    {
        if (idtype.type == OpenVac::CellType::KeyEdge)
        {
            createVBO_(idtype.id);
            updateVBO_(idtype.id);
        }
    }

    // Loop over all key edges affected by topology edits, and update corresponding VBO
    //
    for (OpenVac::CellIdType idtype: topologyEditInfo_.affected())
    {
        if (idtype.type == OpenVac::CellType::KeyEdge)
        {
            updateVBO_(idtype.id);
        }
    }

    // Loop over all key edges affected by geometry edits, and update corresponding VBO
    //
    for (OpenVac::CellIdType idtype: geometryEditInfo_.affected())
    {
        if (idtype.type == OpenVac::CellType::KeyEdge)
        {
            updateVBO_(idtype.id);
        }
    }

    // Clear info about topology and geometry edits
    //
    topologyEditInfo_.clear();
    geometryEditInfo_.clear();
}

void VacRendererSharedResources::cleanup(OpenGLFunctions * /*f*/)
{
    --refCount_;

    if (refCount_ == 0)
    {
        // Get all IDs of VBOs currently in use
        std::vector<OpenVac::CellId> ids;
        for(auto pair : keyEdgeGLSharedResources_)
        {
            OpenVac::CellId id = pair.first;
            ids.push_back(id);
        }

        // Destroy all VBOs
        for(OpenVac::CellId id : ids)
        {
            destroyVBO_(id);
        }
    }
}

void VacRendererSharedResources::onTopologyChanged(const OpenVac::TopologyEditInfo & info)
{
    // Store edit info, composing with pre-existing edits, if any
    topologyEditInfo_.compose(info);
}

void VacRendererSharedResources::onGeometryChanged(const OpenVac::GeometryEditInfo & info)
{
    // Store edit info, composing with pre-existing edits, if any
    geometryEditInfo_.compose(info);
}

void VacRendererSharedResources::createVBO_(OpenVac::CellId id)
{
    // Create VBO
    QOpenGLBuffer vbo;
    vbo.create();

    // Insert in map
    KeyEdgeGLSharedResources resources;
    resources.vbo = vbo;
    resources.numVertices = 0;
    keyEdgeGLSharedResources_[id] = resources;
}

void VacRendererSharedResources::updateVBO_(OpenVac::CellId id)
{
    // Get resources
    KeyEdgeGLSharedResources & resources = keyEdgeGLSharedResources_[id];

    // Get VBO
    QOpenGLBuffer & vbo = resources.vbo;

    // Get key edge handle
    OpenVac::KeyEdgeHandle edge = vac()->data().cell(id);
    assert((bool) edge);

    // Get curve
    const VecCurve & curve = edge->geometry().curve();
    const size_t nSamples = curve.numSamples();

    // Convert curve samples to GL samples
    std::vector<EdgeGeometryGLSample> glSamples;
    glSamples.reserve(nSamples);
    for (unsigned int i=0; i<nSamples; ++i)
    {
        const VecCurveSample & sample = curve.sample(i);

        EdgeGeometryGLSample glSample;

        glSample.left.centerline = sample.position;
        glSample.left.normal     = sample.normal;
        glSample.left.position   = sample.position + sample.width * sample.normal;

        glSample.right.centerline = sample.position;
        glSample.right.normal     = - sample.normal;
        glSample.right.position   = sample.position - sample.width * sample.normal;

        glSamples.push_back(glSample);
    }

    // Update VBO (i.e., send data from RAM to GPU)
    vbo.bind();
    vbo.allocate(glSamples.data(), nSamples * sizeof(EdgeGeometryGLSample));
    vbo.release();

    // Remember number of GLVertices in VBO
    resources.numVertices = 2 * nSamples;
}

void VacRendererSharedResources::destroyVBO_(OpenVac::CellId id)
{
    // Get resources
    KeyEdgeGLSharedResources & resources = keyEdgeGLSharedResources_[id];

    // Get VBO
    QOpenGLBuffer & vbo = resources.vbo;

    // Destroy VBO
    vbo.destroy();

    // Erase from map
    keyEdgeGLSharedResources_.erase(id);
}
