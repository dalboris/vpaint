// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "Scene/SceneRendererSharedResources.h"

#include "Scene/Scene.h"
#include "Scene/SceneData.h"

SceneRendererSharedResources::SceneRendererSharedResources(
        Scene * scene,
        QObject * parent) :
    QObject(parent),
    scene_(scene)
{
    connect(scene_, &Scene::changed, this, &SceneRendererSharedResources::setDirty);
}

void SceneRendererSharedResources::setDirty()
{
    isDirty_ = true;
}

void SceneRendererSharedResources::initialize(OpenGLFunctions * f)
{
    if (refCount_ == 0)
    {
        // Create shader program
        shaderProgram_.reset(new QOpenGLShaderProgram());
        shaderProgram_->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/OpenGL/Shaders/Helloworld.v.glsl");
        shaderProgram_->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/OpenGL/Shaders/Helloworld.f.glsl");
        shaderProgram_->link();

        // Get shader locations
        shaderProgram_->bind();
        vertexLoc_     = shaderProgram_->attributeLocation("vertex");
        projMatrixLoc_ = shaderProgram_->uniformLocation("projMatrix");
        viewMatrixLoc_ = shaderProgram_->uniformLocation("viewMatrix");
        shaderProgram_->release();

        // Create VBO
        vbo_.create();
    }

    ++refCount_;
}

void SceneRendererSharedResources::update(OpenGLFunctions * f)
{
    if (isDirty_)
    {
        const auto & samples = scene()->samples();
        vbo_.bind();
        vbo_.allocate(samples.data(), samples.size() * sizeof(SceneDataSample));
        vbo_.release();
    }

    isDirty_ = false;
}

void SceneRendererSharedResources::cleanup(OpenGLFunctions * /*f*/)
{
    --refCount_;

    if (refCount_ == 0)
    {
        shaderProgram_.reset();
        vbo_.destroy();
    }
}
