// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "Vac/Vac.h"
#include "Vac/VacRenderer.h"
#include "Vac/VacRendererSharedResources.h"

#include "OpenVac/Topology/KeyEdge.h"

VacRenderer::VacRenderer(VacRendererSharedResources * sharedResources) :
    sharedResources_(sharedResources)
{
    connect(vac(), &Vac::topologyChanged, this, &VacRenderer::onTopologyChanged);
    connect(vac(), &Vac::geometryChanged, this, &VacRenderer::onGeometryChanged);
}

VacRenderer::~VacRenderer()
{
}

VacRendererSharedResources * VacRenderer::sharedResources() const
{
    return sharedResources_;
}

Vac * VacRenderer::vac() const
{
    return sharedResources()->vac();
}

void VacRenderer::initialize(OpenGLFunctions * f)
{
    // Initialize shared resources
    sharedResources()->initialize(f);

    // Set clear color (XXX TODO move to SceneRenderer)
    glClearColor(1, 1, 1, 1);
}

void VacRenderer::update(OpenGLFunctions * f)
{
    // Update shared resources
    //
    sharedResources()->update(f);

    // Loop over all destroyed key edges, and destroy corresponding VBO
    //
    for (OpenVac::CellIdType idtype: topologyEditInfo_.destroyed())
    {
        if (idtype.type == OpenVac::CellType::KeyEdge)
        {
            destroyVAO_(f, idtype.id);
        }
    }

    // Loop over all created key edges, and create one VBO for each
    //
    for (OpenVac::CellIdType idtype: topologyEditInfo_.created())
    {
        if (idtype.type == OpenVac::CellType::KeyEdge)
        {
            createVAO_(f, idtype.id);
            updateVAO_(f, idtype.id);
        }
    }

    // Loop over all key edges affected by topology edits, and update corresponding VBO
    //
    for (OpenVac::CellIdType idtype: topologyEditInfo_.affected())
    {
        if (idtype.type == OpenVac::CellType::KeyEdge)
        {
            updateVAO_(f, idtype.id);
        }
    }

    // Loop over all key edges affected by geometry edits, and update corresponding VBO
    //
    for (OpenVac::CellIdType idtype: geometryEditInfo_.affected())
    {
        if (idtype.type == OpenVac::CellType::KeyEdge)
        {
            updateVAO_(f, idtype.id);
        }
    }

    // Clear info about topology and geometry edits
    //
    topologyEditInfo_.clear();
    geometryEditInfo_.clear();
}

void VacRenderer::render2D(OpenGLFunctions * f, const QMatrix4x4 & projMatrix, const QMatrix4x4 & viewMatrix)
{
    update(f);

    // Get shared resources
    auto & shaderProgram = sharedResources_->shaderProgram_;
    auto & projMatrixLoc = sharedResources_->projMatrixLoc_;
    auto & viewMatrixLoc = sharedResources_->viewMatrixLoc_;

    // Clear color and depth buffer (XXX TODO move to SceneRenderer)
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Bind shader program
    shaderProgram.bind();

    // Set uniform values
    shaderProgram.setUniformValue(projMatrixLoc, projMatrix);
    shaderProgram.setUniformValue(viewMatrixLoc, viewMatrix);

    // Draw all key edges
    for (auto & pair : vaos_)
    {
        QOpenGLVertexArrayObject & vao = *pair.second.vao;
        size_t & numIndices = pair.second.numIndices;

        vao.bind();
        f->glDrawArrays(GL_TRIANGLE_STRIP, // mode
                        0,                 // first index
                        numIndices);       // number of indices
        vao.release();
    }

    // Release shader program
    shaderProgram.release();
}

void VacRenderer::render3D(OpenGLFunctions * /*f*/)
{
    // XXX TODO
}

void VacRenderer::cleanup(OpenGLFunctions * f)
{
    // Cleanup shared resources
    sharedResources()->cleanup(f);

    // Get all IDs of VAOs currently in use
    std::vector<OpenVac::CellId> ids;
    for(auto & pair : vaos_)
    {
        OpenVac::CellId id = pair.first;
        ids.push_back(id);
    }

    // Destroy all VBOs
    for(OpenVac::CellId id : ids)
    {
        destroyVAO_(f, id);
    }
}

void VacRenderer::onTopologyChanged(const OpenVac::TopologyEditInfo & info)
{
    // Store edit info, composing with pre-existing edits, if any
    topologyEditInfo_.compose(info);
}

void VacRenderer::onGeometryChanged(const OpenVac::GeometryEditInfo & info)
{
    // Store edit info, composing with pre-existing edits, if any
    geometryEditInfo_.compose(info);
}

void VacRenderer::createVAO_(OpenGLFunctions * f, OpenVac::CellId id)
{
    // Create VAO
    DrawKeyCellInfo info;
    info.vao = new QOpenGLVertexArrayObject();
    QOpenGLVertexArrayObject & vao = *info.vao;
    vao.create();

    // Store attribute bindings in VAO
    auto & vbo = sharedResources_->vbos_.at(id);
    auto & vertexLoc = sharedResources_->vertexLoc_;
    GLsizei  stride  = sizeof(EdgeGeometryGpuVertex);
    GLvoid * pointer = reinterpret_cast<void*>(offsetof(EdgeGeometryGpuVertex, position));
    vao.bind();
    vbo.bind();
    f->glEnableVertexAttribArray(vertexLoc);
    f->glVertexAttribPointer(
                vertexLoc, // index of the generic vertex attribute
                2,         // number of components   (x and y components)
                GL_FLOAT,  // type of each component
                GL_FALSE,  // should it be normalized
                stride,    // byte offset between consecutive vertex attributes
                pointer);  // byte offset between the first attribute and the pointer given to allocate()
    vbo.release();
    vao.release();

    // Insert in map
    vaos_[id] = info;
}

void VacRenderer::updateVAO_(OpenGLFunctions * /*f*/, OpenVac::CellId id)
{
    // Get VAO info
    DrawKeyCellInfo & info = vaos_[id];

    // Get key edge handle
    OpenVac::KeyEdgeHandle edge = vac()->data().cell(id);
    assert((bool) edge);

    // Get edge geometry samples
    const auto & samples = edge->geometry().samples();
    int numIndices = samples.size() * 2;

    // Update VAO info
    info.numIndices = numIndices;
}

void VacRenderer::destroyVAO_(OpenGLFunctions * /*f*/, OpenVac::CellId id)
{
    // Get VAO
    QOpenGLVertexArrayObject * vaoPtr = vaos_[id].vao;
    QOpenGLVertexArrayObject & vao = *vaoPtr;

    // Destroy VAO
    vao.destroy();
    delete vaoPtr;

    // Erase from map
    vaos_.erase(id);
}
