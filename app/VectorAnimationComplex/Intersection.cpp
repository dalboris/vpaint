// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "Intersection.h"

namespace VectorAnimationComplex
{

// returns if there is an intersection or not
// in addition, return in s and t the position of intersection:
//   I = A + s*(B-A)/||B-A|| = C + t*(D-C)/||D-C||
//
//   In a math world, 0 <= s <= ||B-A|| and 0 <= t <= ||D-C||
//
//   Here, sometimes returns true with -epsilon < s < 0
//   or ||B-A|| < s < ||B-A|| + epsilon
//
//   This is  to be sure we  don't miss any  intersection due to
//   rounding   error,  but   caution  then,   maybe   the  same
//   intersection could be found twice.
bool segmentIntersection(
    const Eigen::Vector2d & a, const Eigen::Vector2d & b,
    const Eigen::Vector2d & c, const Eigen::Vector2d & d,
    double dsSquared, double &s, double &t)
{
    // avoid useless computation if too far
    if((c-a).squaredNorm() < dsSquared)
    {
        // cf page 54 of my written notes
        Eigen::Vector2d u = (b-a);
        double normU = u.norm();
        if(normU > 0)
            u = (1/normU) * u;
        Eigen::Vector2d v = (d-c);
        double normV = v.norm();
        if(normV > 0)
            v = (1/normV)*v;

        double detA = v[0]*u[1]-v[1]*u[0];
        double epsilon = 1e-10;
        // reason of epsilon:
        //   don't intersect if nearly parallel
        if(detA < -epsilon || epsilon < detA)
        {
            Eigen::Matrix2d invA;
            invA << -v[1], v[0],
                -u[1], u[0];
            invA = (1/detA)*invA;
            Eigen::Vector2d st = invA*(c-a);

            // reason of eps:
            //   can't miss an intersection due
            //   to rounding errors, but eventually
            //   find the same twice.
            double eps = 1e-10;
            if(-eps <= st[0] && st[0] < normU + eps &&
               -eps <= st[1] && st[1] < normV + eps)
            {
                // here, we know they intersect
                s = st[0];
                t = st[1];
                return true;
            }
        }
    }

    return false;
}

}
