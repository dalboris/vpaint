// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "Scene/Scene.h"
#include "Scene/SceneRenderer.h"
#include "Scene/SceneRendererSharedResources.h"

SceneRenderer::SceneRenderer(SceneRendererSharedResources * sharedResources,
                             QObject * parent) :
    QObject(parent),
    sharedResources_(sharedResources)
{
}

Scene * SceneRenderer::scene() const
{
    return sharedResources_->scene();
}

void SceneRenderer::initialize(OpenGLFunctions * f)
{
    // Initialize shared resources
    sharedResources_->initialize(f);

    // Set clear color
    glClearColor(1, 1, 1, 1);

    // Create VAO
    vao_.create();

    // Store attribute bindings in VAO
    auto & vbo = sharedResources_->vbo_;
    auto & vertexLoc = sharedResources_->vertexLoc_;
    GLsizei  stride  = sizeof(SceneDataSample) / 2;
    GLvoid * pointer = reinterpret_cast<void*>(offsetof(SceneDataSample, leftBoundary));
    vao_.bind();
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
    vao_.release();

}

void SceneRenderer::render2D(OpenGLFunctions * f, const QMatrix4x4 & projMatrix, const QMatrix4x4 & viewMatrix)
{
    // Update shared resources
    sharedResources_->update(f);

    // Get shared resources
    auto & shaderProgram = sharedResources_->shaderProgram_;
    auto & projMatrixLoc = sharedResources_->projMatrixLoc_;
    auto & viewMatrixLoc = sharedResources_->viewMatrixLoc_;
    const auto & samples = scene()->samples();

    // Clear color and depth buffer
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Bind shader program
    shaderProgram.bind();

    // Set uniform values
    shaderProgram.setUniformValue(projMatrixLoc, projMatrix);
    shaderProgram.setUniformValue(viewMatrixLoc, viewMatrix);

    // Draw triangles
    vao_.bind();
    f->glDrawArrays(GL_TRIANGLE_STRIP,   // mode
                    0,                   // starting index
                    samples.size() * 2); // number of indices
    vao_.release();

    // Release shader program
    shaderProgram.release();
}

void SceneRenderer::render3D(OpenGLFunctions * /*f*/)
{
    // XXX TODO
}

void SceneRenderer::cleanup(OpenGLFunctions * f)
{
    // Cleanup shared resources
    sharedResources_->cleanup(f);

    // Cleanup context-specific resources
    vao_.destroy();
}
