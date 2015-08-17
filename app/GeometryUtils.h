// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

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
