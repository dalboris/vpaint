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

#ifndef GEOMETRYUTILS_H
#define GEOMETRYUTILS_H

#include <QPointF>
#include <cmath>
#include <Eigen/Core>
#include "vpaint_global.h"

class Q_VPAINT_EXPORT GeometryUtils
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
