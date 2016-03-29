// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "OpenGL/OpenGLRenderer.h"

OpenGLRenderer::OpenGLRenderer(QObject * parent) :
    QObject(parent)
{
}

const QMatrix4x4 & OpenGLRenderer::projectionMatrix() const
{
    return projMatrix_;
}

const QMatrix4x4 & OpenGLRenderer::projectionMatrixInverse() const
{
    if (projMatrixInv_.isDirty())
    {
        projMatrixInv_.setValue(projMatrix_.inverted());
    }
    return projMatrixInv_.value();
}

void OpenGLRenderer::setProjectionMatrix(const QMatrix4x4 & projectionMatrix)
{
    projMatrix_ = projectionMatrix;
    projMatrixInv_.setDirty();
}

const QMatrix4x4 & OpenGLRenderer::viewMatrix() const
{
    return viewMatrix_;
}

const QMatrix4x4 & OpenGLRenderer::viewMatrixInverse() const
{
    if (viewMatrixInv_.isDirty())
    {
        viewMatrixInv_.setValue(viewMatrix_.inverted());
    }
    return viewMatrixInv_.value();
}

void OpenGLRenderer::setViewMatrix(const QMatrix4x4 & viewMatrix)
{
    viewMatrix_ = viewMatrix;
    viewMatrixInv_.setDirty();
}
