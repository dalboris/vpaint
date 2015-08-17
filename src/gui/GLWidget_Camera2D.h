// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

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
