// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "Camera2D.h"

Camera2D::Camera2D()
{
}

QMatrix4x4 Camera2D::toMatrix() const
{
    return data().toMatrix();
}

QPointF Camera2D::position() const
{
    return data().position;
}

void Camera2D::setPosition(const QPointF & p)
{
    data_.position = p;
    emit changed();
}

double Camera2D::rotation() const
{
    return data().rotation;
}

void Camera2D::setRotation(double r)
{
    data_.rotation = r;
    emit changed();
}

double Camera2D::scale() const
{
    return data().scale;
}

void Camera2D::setScale(double s)
{
    data_.scale = s;
    emit changed();
}
