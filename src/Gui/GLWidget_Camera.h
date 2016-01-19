// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef GLWIDGET_CAMERA_H
#define GLWIDGET_CAMERA_H

#include <Eigen/Core>
#include <Eigen/Geometry>

/* 
 * The Camera is defined by:
 *   - The position of its focus point
 *   - Its distance and angle around this point
 *   - Its aperture
 *
 *   There are only 2 angles to define the orientation of the camera: yaw and
 *   pitch (no roll). This is a voluntary loss of degree of freedom: we force
 *   the horizon line to be horizontal on screen (avoid getting lost and/or
 *   sea-sick).
 *
 */

class GLWidget_Camera
{
public:
    // initialization: correctly positionned to see a unit cube
    GLWidget_Camera() :
        r_(4), theta_(3.14/10), phi_(3.14/6),
        focus_x_(0), focus_y_(0), focus_z_(0),
        fovy_(3.14/4)
    {
        setDirty_();
    }

    // Camera position and orientation (= View Matrix)
    double r()       const { return r_;       }
    double theta()   const { return theta_;   }
    double phi()     const { return phi_;     }
    double focus_x() const { return focus_x_; }
    double focus_y() const { return focus_y_; }
    double focus_z() const { return focus_z_; }

    // Camera field of view (= Projection Matrix)
    double fovy()    const { return fovy_;    }

    // Setters
    void setR(double r)             { r_=r;             setDirty_(); }
    void setTheta(double theta)     { theta_=theta;     setDirty_(); }
    void setPhi(double phi)         { phi_=phi;         setDirty_(); }
    void setFocus_x(double focus_x) { focus_x_=focus_x; setDirty_(); }
    void setFocus_y(double focus_y) { focus_y_=focus_y; setDirty_(); }
    void setFocus_z(double focus_z) { focus_z_=focus_z; setDirty_(); }
    void setFovy(double fovy)       { fovy_=fovy;       setDirty_(); }

    // Get matrices
    Eigen::Affine3d viewMatrix() const
    {
        computeViewMatrix_();
        return viewMatrix_;
    }

    Eigen::Affine3d viewMatrixInverse() const
    {
        computeViewMatrixInverse_();
        return viewMatrixInverse_;
    }

    // convenient methods to access other geometric properties
    Eigen::Vector3d position() const
    {
        double cos_theta = std::cos(theta());
        double sin_theta = std::sin(theta());
        double cos_phi   = std::cos(phi());
        double sin_phi   = std::sin(phi());

        return Eigen::Vector3d(
                    focus_x() + r() * cos_theta * sin_phi,
                    focus_y() + r() * sin_theta,
                    focus_z() + r() * cos_theta * cos_phi);
    }

    Eigen::Vector3d focusPoint() const
    {
        return Eigen::Vector3d(focus_x(),    focus_y(), focus_z());
    }

    Eigen::Vector3d upDirection() const
    {
        double cos_theta = std::cos(theta());
        double sin_theta = std::sin(theta());
        double cos_phi   = std::cos(phi());
        double sin_phi   = std::sin(phi());

        return Eigen::Vector3d(
                    - sin_theta * sin_phi,
                    cos_theta,
                    - sin_theta * cos_phi);
    }

    
private:
    // Camera parameters
    double r_;
    double theta_;
    double phi_;
    double focus_x_;
    double focus_y_;
    double focus_z_;
    double fovy_;

    // Matrix
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
            // [...] compute it
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


#endif
