// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef GEOMETRYUTILS_H
#define GEOMETRYUTILS_H

#include <QPointF>
#include <cmath>
#include <Eigen/Core>

class GeometryUtils
{
public:
    static double length(QPointF p){return sqrt(p.x()*p.x()+p.y()*p.y());}
    static double dot(QPointF p, QPointF q){return p.x()*q.x()+p.y()*q.y();}
    static double det(QPointF p, QPointF q){return p.x()*q.y()-p.y()*q.x();}
    // return a value in [0..4[ bijective with [0..2Pi[
    static double angleLike(QPointF u, QPointF v);
    // same with v=(1,0), assumes u is unitary
    static double angleLike(const Eigen::Vector2d & u);
    static double angleLike(const Eigen::Vector2d & u, const Eigen::Vector2d & v);
    static bool segmentIntersects(QPointF a, QPointF b, QPointF c, QPointF d);
    
};

    
#endif
