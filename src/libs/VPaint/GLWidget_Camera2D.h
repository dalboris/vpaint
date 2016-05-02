// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef GLWIDGET_CAMERA2D_H
#define GLWIDGET_CAMERA2D_H

#include <Eigen/Core>
#include <Eigen/Geometry>

// for now, no rotation

class GLWidget_Camera2D
{
public:
    // initialization: correctly positionned to see a unit cube
    GLWidget_Camera2D() : x_(0), y_(0), zoom_(1.0)
    {
        setDirty_();
    }

    // Getters
    double x()    const { return x_;    }
    double y()    const { return y_;    }
    double zoom() const { return zoom_; }

    // Setters
    void setX(double x)       { x_=x;       setDirty_(); }
    void setY(double y)       { y_=y;       setDirty_(); }
    void setZoom(double zoom) { zoom_=zoom; setDirty_(); }

    // Get matrices
    Eigen::Affine3d viewMatrix() const
    {
        computeViewMatrix_();
        return viewMatrix_;
    }
    double * viewMatrixData()
    {
        computeViewMatrix_();
        return viewMatrix_.data();
    }

    Eigen::Affine3d viewMatrixInverse() const
    {
        computeViewMatrixInverse_();
        return viewMatrixInverse_;
    }

private:
    // Camera parameters
    double x_;
    double y_;
    double zoom_;

    // Matrices
    mutable bool viewMatrixDirty_;
    mutable bool viewMatrixInverseDirty_;
    mutable Eigen::Affine3d viewMatrix_;
    mutable Eigen::Affine3d viewMatrixInverse_;
    void setDirty_()
    {
        viewMatrixDirty_ = true;
        viewMatrixInverseDirty_ = true;
    }
    void computeViewMatrix_() const
    {
        if(viewMatrixDirty_)
        {
            viewMatrix_ = Eigen::Translation<double,3>(x_, y_, 0) * Eigen::Scaling(zoom_);
            viewMatrixDirty_ = false;
        }
    }
    void computeViewMatrixInverse_() const
    {
        if(viewMatrixInverseDirty_)
        {
            computeViewMatrix_();
            viewMatrixInverse_ = viewMatrix_.inverse();
            viewMatrixInverseDirty_ = false;
        }
    }

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

#endif // GLWIDGET_CAMERA2D_H
