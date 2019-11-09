// Copyright (C) 2012-2019 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "GeometryUtils.h"

double GeometryUtils::angleLike(QPointF u, QPointF v)
{
    double lu=length(u);
    if(lu>0) u = 1/lu * u;
    double lv=length(v);
    if(lv>0) v = 1/lv * v;

    double dot_ = dot(u,v);
    double det_ = det(u,v);
    double signDet = 1;
    if(det_ < 0) signDet = -1;
    return 2 - signDet*(dot_+1);
}

bool GeometryUtils::segmentIntersects(QPointF a, QPointF b, QPointF c, QPointF d)
{
    double det_ = det(b-a, d-c);
    double numR = det(d-c, a-c);
    double numS = det(b-a, a-c);

    // if parallel
    if(det_ == 0) // should be replace by abs(det_) < epsilon
    {
        return false;
        // something better could be done
        // if numR  == 0 also,  ab and cd are  collinear, possibly
        // infinite intersection exists
    }
    else
    {
        double invDet = 1/det_;
        double r = numR * invDet;
        double s = numS * invDet;
        if(r>=0 && r<1 && s>=0 && s<1)
           return true;
        else
           return false;
    }
}

double GeometryUtils::angleLike(const Eigen::Vector2d & u)
{
    // assume u is unitary
    if(u[1]>=0)
        return 1-u[0];
    else
        return 3+u[0];
}


double GeometryUtils::angleLike(const Eigen::Vector2d & u, const Eigen::Vector2d & v)
{
    // assume u and v are unitary
    /*
    double lu = length(u);
    if(lu>0) u = 1/lu * u;
    double lv=length(v);
    if(lv>0) v = 1/lv * v;
    */
    
    double dot_ = u.dot(v);
    double det_ = u[0]*v[1] - u[1]*v[0];
    double signDet = 1;
    if(det_ < 0) signDet = -1;
    return 2 - signDet*(dot_+1);
}
