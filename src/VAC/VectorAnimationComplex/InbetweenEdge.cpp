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

#include "InbetweenEdge.h"
#include "KeyVertex.h"
#include "KeyEdge.h"
#include "InbetweenVertex.h"
#include "Halfedge.h"
#include "EdgeGeometry.h"
#include "VAC.h"

#include "Eigen.h"
#include <QPair>
#include <QSet>
#include "../OpenGL.h"
#include <QtDebug>
#include <QTextStream>
#include "../SaveAndLoad.h"
#include "../DevSettings.h"
#include "../Global.h"

#include "../XmlStreamWriter.h"
#include "../XmlStreamReader.h"

#include <assert.h>

namespace VectorAnimationComplex
{

bool InbetweenEdge::isClosed() const
{
    return !startAnimatedVertex_.isValid();
}

void InbetweenEdge::setBeforeCycleStartingPoint(double s0)
{
    beforeCycle_.setStartingPoint(s0);
    processGeometryChanged_();
}

void InbetweenEdge::setAfterCycleStartingPoint(double s0)
{
    afterCycle_.setStartingPoint(s0);
    processGeometryChanged_();
}

double InbetweenEdge::beforeCycleStartingPoint() const
{
    return beforeCycle_.s0();
}

double InbetweenEdge::afterCycleStartingPoint() const
{
    return afterCycle_.s0();
}

InbetweenEdge::InbetweenEdge(VAC * vac,
                             const Path & beforePath,
                             const Path & afterPath,
                             const AnimatedVertex & startAnimatedVertex,
                             const AnimatedVertex & endAnimatedVertex) :
    Cell(vac),
    InbetweenCell(vac),
    EdgeCell(vac),

    beforePath_(beforePath),
    afterPath_(afterPath),
    startAnimatedVertex_(startAnimatedVertex),
    endAnimatedVertex_(endAnimatedVertex)

{
    // Check pre-conditions
    assert(beforePath_.isValid());
    assert(afterPath_.isValid());
    assert(beforePath_.time() < afterPath_.time());
    assert(startAnimatedVertex_.isValid());
    assert(endAnimatedVertex_.isValid());
    assert(beforePath_.startVertex() == startAnimatedVertex_.beforeVertex());
    assert(afterPath_.startVertex() == startAnimatedVertex_.afterVertex());
    assert(beforePath_.endVertex() == endAnimatedVertex_.beforeVertex());
    assert(afterPath_.endVertex() == endAnimatedVertex_.afterVertex());

    // Cache star
    for(VertexCell * vertex: startVertices())
        addMeToSpatialStarOf_(vertex);
    for(VertexCell * vertex: endVertices())
        addMeToSpatialStarOf_(vertex);
    for(KeyCell * kcell: beforeCells())
        addMeToTemporalStarAfterOf_(kcell);
    for(KeyCell * kcell: afterCells())
        addMeToTemporalStarBeforeOf_(kcell);
}

InbetweenEdge::InbetweenEdge(VAC * vac,
                             const Cycle & beforeCycle,
                             const Cycle & afterCycle) :
    Cell(vac),
    InbetweenCell(vac),
    EdgeCell(vac),

    beforeCycle_(beforeCycle),
    afterCycle_(afterCycle)
{
    // Check pre-conditions
    assert(beforeCycle_.isValid());
    assert(afterCycle_.isValid());
    assert(beforeCycle_.time() < afterCycle_.time());

    // Cache star
    for(KeyCell * kcell: beforeCells())
        addMeToTemporalStarAfterOf_(kcell);
    for(KeyCell * kcell: afterCells())
        addMeToTemporalStarBeforeOf_(kcell);
}

InbetweenEdge::~InbetweenEdge()
{
}



InbetweenEdge::InbetweenEdge(VAC * g, QTextStream & in) :
    Cell(g,in),
    InbetweenCell(g, in),
    EdgeCell(g, in)
{
    Field field;

    // Before Path
    in >> field >> beforePath_;

    // After Path
    in >> field >> afterPath_;

    // Start Animated Vertex
    in >> field >> startAnimatedVertex_;

    // End Animated Vertex
    in >> field >> endAnimatedVertex_;

    // Before Cycle
    in >> field >> beforeCycle_;

    // After Cycle
    in >> field >> afterCycle_;
}

void InbetweenEdge::read2ndPass()
{
    // Base classes
    Cell::read2ndPass();
    InbetweenCell::read2ndPass();
    EdgeCell::read2ndPass();

    // Before Path
    beforePath_.convertTempIdsToPointers(vac());

    // After Path
    afterPath_.convertTempIdsToPointers(vac());

    // Start Animated Vertex
    startAnimatedVertex_.convertTempIdsToPointers(vac());

    // End Animated Vertex
    endAnimatedVertex_.convertTempIdsToPointers(vac());

    // Before Cycle
    beforeCycle_.convertTempIdsToPointers(vac());

    // After Cycle
    afterCycle_.convertTempIdsToPointers(vac());
}

void InbetweenEdge::save_(QTextStream & out)
{
        // Base classes
        Cell::save_(out);
        InbetweenCell::save_(out);
        EdgeCell::save_(out);

        // Before Path
        out << Save::newField("BeforePath") << beforePath_;

        // After Path
        out << Save::newField("AfterPath") << afterPath_;

        // Start Animated Vertex
        out << Save::newField("StartAnimatedVertex") << startAnimatedVertex_;

        // End Animated Vertex
        out << Save::newField("EndAnimatedVertex") << endAnimatedVertex_;

        // Before Cycle
        out << Save::newField("BeforeCycle") << beforeCycle_;

        // After Cycle
        out << Save::newField("AfterCycle") << afterCycle_;
}

QString InbetweenEdge::xmlType_() const
{
    return "inbetweenedge";
}

void InbetweenEdge::write_(XmlStreamWriter & xml) const
{
    // Base classes
    Cell::write_(xml);
    InbetweenCell::write_(xml);
    EdgeCell::write_(xml);

    if(isClosed())
    {
        // Before/After cycle
        xml.writeAttribute("beforecycle", beforeCycle_.toString());
        xml.writeAttribute("aftercycle", afterCycle_.toString());

        // Cycle offset
        if(afterCycle_.s0() != 0.0)
            xml.writeAttribute("cycleoffset", QString().setNum(afterCycle_.s0()));
    }
    else
    {
        // Before/After paths
        xml.writeAttribute("beforepath", beforePath_.toString());
        xml.writeAttribute("afterpath", afterPath_.toString());

        // Start/End animated vertex
        xml.writeAttribute("startanimatedvertex", startAnimatedVertex_.toString());
        xml.writeAttribute("endanimatedvertex", endAnimatedVertex_.toString());
    }
}

InbetweenEdge::InbetweenEdge(VAC * vac, XmlStreamReader & xml) :
    Cell(vac,xml),
    InbetweenCell(vac, xml),
    EdgeCell(vac, xml)
{
    if(xml.attributes().hasAttribute("beforecycle"))
    {
        beforeCycle_.fromString(xml.attributes().value("beforecycle").toString());
        afterCycle_.fromString(xml.attributes().value("aftercycle").toString());

        // Cycle offset
        if(xml.attributes().hasAttribute("cycleoffset"))
            afterCycle_.setStartingPoint(xml.attributes().value("cycleoffset").toDouble());
    }
    else
    {
        beforePath_.fromString(xml.attributes().value("beforepath").toString());
        afterPath_.fromString(xml.attributes().value("afterpath").toString());

        startAnimatedVertex_.fromString(xml.attributes().value("startanimatedvertex").toString());
        endAnimatedVertex_.fromString(xml.attributes().value("endanimatedvertex").toString());
    }
}


    InbetweenEdge * InbetweenEdge::clone()
    {
        return new InbetweenEdge(this);
    }

    void InbetweenEdge::remapPointers(VAC * newVAC)
    {
        Cell::remapPointers(newVAC);
        InbetweenCell::remapPointers(newVAC);
        EdgeCell::remapPointers(newVAC);

        beforePath_.remapPointers(newVAC);
        afterPath_.remapPointers(newVAC);

        startAnimatedVertex_.remapPointers(newVAC);
        endAnimatedVertex_.remapPointers(newVAC);

        beforeCycle_.remapPointers(newVAC);
        afterCycle_.remapPointers(newVAC);
    }

    InbetweenEdge::InbetweenEdge(InbetweenEdge * other) :
        Cell(other),
        InbetweenCell(other),
        EdgeCell(other)
    {
        beforePath_ = other->beforePath_;
        afterPath_ = other->afterPath_;

        startAnimatedVertex_ = other->startAnimatedVertex_;
        endAnimatedVertex_ = other->endAnimatedVertex_;

        beforeCycle_ = other->beforeCycle_;
        afterCycle_ = other->afterCycle_;
    }

    VertexCell * InbetweenEdge::startVertex(Time time) const
    {
        for(VertexCell * v: startVertices())
            if(v->exists(time))
                return v;
        return 0;
    }

    VertexCell * InbetweenEdge::endVertex(Time time) const
    {
        for(VertexCell * v: endVertices())
            if(v->exists(time))
                return v;
        return 0;
    }

    // Update Boundary
    void InbetweenEdge::updateBoundary_impl(KeyVertex * oldVertex, KeyVertex * newVertex)
    {
        beforePath_.replaceVertex(oldVertex,newVertex);
        afterPath_.replaceVertex(oldVertex,newVertex);
        beforeCycle_.replaceVertex(oldVertex,newVertex);
        afterCycle_.replaceVertex(oldVertex,newVertex);
        startAnimatedVertex_.replaceVertex(oldVertex,newVertex);
        endAnimatedVertex_.replaceVertex(oldVertex,newVertex);
    }
    void InbetweenEdge::updateBoundary_impl(const KeyHalfedge & oldHalfedge, const KeyHalfedge & newHalfedge)
    {
        beforePath_.replaceHalfedge(oldHalfedge,newHalfedge);
        afterPath_.replaceHalfedge(oldHalfedge,newHalfedge);
        beforeCycle_.replaceHalfedge(oldHalfedge,newHalfedge);
        afterCycle_.replaceHalfedge(oldHalfedge,newHalfedge);
    }
    void InbetweenEdge::updateBoundary_impl(KeyEdge * oldEdge, const KeyEdgeList & newEdges)
    {
        beforePath_.replaceEdges(oldEdge,newEdges);
        afterPath_.replaceEdges(oldEdge,newEdges);
        beforeCycle_.replaceEdges(oldEdge,newEdges);
        afterCycle_.replaceEdges(oldEdge,newEdges);
    }


    // Drawing
    void InbetweenEdge::glColor3D_()
    {
    }

    void InbetweenEdge::clearCachedGeometry_()
    {
        EdgeCell::clearCachedGeometry_();
        surf_.clear();
        norm_.clear();
    }

    void InbetweenEdge::computeInbetweenSurface(View3DSettings & viewSettings)
    {
        if(!surf_.isEmpty() &&
           cacheSpaceScale_ == viewSettings.spaceScale() &&
           cacheTimeScale_ == viewSettings.timeScale() &&
           cacheK1_ == viewSettings.k1() &&
           cacheK2_ == viewSettings.k2())
        {
            return;
        }

        surf_.clear();
        norm_.clear();
        cacheSpaceScale_ = viewSettings.spaceScale();
        cacheTimeScale_ = viewSettings.timeScale();
        cacheK1_ = viewSettings.k1();
        cacheK2_ = viewSettings.k2();

        double eps = 1e-5;
        double tMin = beforeTime().floatTime();
        double tMax = afterTime().floatTime();
        double dt = 1 / (double)cacheK1_;

        // positions
        for(double t=tMin; t<tMax+eps; t+=dt)
        {
            QList<Eigen::Vector3d> geo3D;

            QList<Eigen::Vector2d> geo2D = getGeometry(Time(t));
            for(int i=0; i< geo2D.size(); ++i)
            {
                Eigen::Vector3d pos(
                        viewSettings.xFromX2D(geo2D[i][0]),
                        viewSettings.yFromY2D(geo2D[i][1]),
                        viewSettings.zFromT(t));
                geo3D << pos;
            }

            surf_ << geo3D;
        }


        if(surf_.size() < 2)
            return;

        // normals
        for(int i=0; i<surf_.size(); i++)
        {
            QList<Eigen::Vector3d> list;
            for(int j=0; j<surf_[0].size(); j++)
            {
                int i_=i;
                int j_=j;
                if(i==surf_.size()-1)
                    i_--;
                if(j==surf_[0].size()-1)
                    j_--;
                Eigen::Vector3d a = surf_[i_][j_];
                Eigen::Vector3d b = surf_[i_][j_+1];
                Eigen::Vector3d c = surf_[i_+1][j_];
                Eigen::Vector3d u = b-a;
                Eigen::Vector3d v = c-a;
                Eigen::Vector3d N = -u.cross(v);
                list << N;
                //qDebug() << N[0] << N[1] << N[2];
            }
            norm_ << list;
        }
    }

    void InbetweenEdge::drawRaw3D(View3DSettings & viewSettings)
    {
        computeInbetweenSurface(viewSettings);

        // drawn in  backward order  to improve the  likeliness the
        // polygon are  drawn from rear  to near, and  improve the
        // chance to get the transparency right
        int n = surf_[0].size();
        for(int i=surf_.size()-2; i>=0; i--)
        {
            glBegin(GL_QUAD_STRIP);
            for(int j=0; j<n;)
            {
                //glColor4d(1.0,0.5,0.5,0.2);
                glNormal3d(norm_[i][j][0],norm_[i][j][1],norm_[i][j][2]);
                glVertex3d(surf_[i][j][0],surf_[i][j][1],surf_[i][j][2]);
                glNormal3d(norm_[i+1][j][0],norm_[i+1][j][1],norm_[i+1][j][2]);
                glVertex3d(surf_[i+1][j][0],surf_[i+1][j][1],surf_[i+1][j][2]);

                if(j == n-1)
                {
                    break;
                }
                else
                {
                    j += cacheK2_;
                    if(j >= n)
                        j = n-1;
                }
            }
            glEnd();
        }
    }

    QList<Eigen::Vector2d>  InbetweenEdge::getGeometry(Time time)
    {

        // Compute lengths of key paths
        double beforeLength = 0;
        double afterLength = 0;
        if(isClosed())
        {
            beforeLength = beforeCycle_.length();
            afterLength = afterCycle_.length();
        }
        else
        {
            beforeLength = beforePath_.length();
            afterLength = afterPath_.length();
        }
        double maxLength = std::max(beforeLength,afterLength);
        // Compute uniform sampling of key paths
        int minSamples = isClosed() ? 4 : 2;
        int numSamples = std::max(minSamples, (int) (maxLength/5.0) + 2);
        QList<Eigen::Vector2d> beforeSampling;
        QList<Eigen::Vector2d> afterSampling;
        if(isClosed())
        {
            beforeCycle_.sample(numSamples,beforeSampling);
            afterCycle_.sample(numSamples,afterSampling);
        }
        else
        {
            beforePath_.sample(numSamples,beforeSampling);
            afterPath_.sample(numSamples,afterSampling);
        }
        assert(beforeSampling.size() == numSamples);
        assert(afterSampling.size() == numSamples);
        // Interpolate key paths
        double t = time.floatTime(); // in [t1,t2]
        double t1 = beforeTime().floatTime();
        double t2 = afterTime().floatTime();
        double dt = t2-t1;
        double u; // in [0,1]
        if(dt > 0)
            u = (t-t1)/dt;
        else if (t<t1)
            u = 0;
        else
            u = 1;
        QList<Eigen::Vector2d> sampling;
        for(int i=0; i<numSamples; ++i)
            sampling << beforeSampling[i] + u * (afterSampling[i]-beforeSampling[i]);
        // Warp to ensure topological constraints
        if(!isClosed())
        {
            Eigen::Vector2d currentStartPos = sampling.first();
            Eigen::Vector2d currentEndPos = sampling.last();
            Eigen::Vector2d desiredStartPos = startAnimatedVertex_.pos(time);
            Eigen::Vector2d desiredEndPos = endAnimatedVertex_.pos(time);
            Eigen::Vector2d deltaStartPos =  desiredStartPos - currentStartPos;
            Eigen::Vector2d deltaEndPos =  desiredEndPos - currentEndPos;
            double du = 1.0/(numSamples-1);
            for(int i=0; i<numSamples; ++i)
            {
                double u = i * du;
                sampling[i] +=  (1-u) * deltaStartPos + u*deltaEndPos;
            }
        }

        return sampling;
    }

    QList<EdgeSample> InbetweenEdge::getSampling(Time time) const
    {
        // Compute lengths of key paths
        double beforeLength = 0;
        double afterLength = 0;
        if(isClosed())
        {
            beforeLength = beforeCycle_.length();
            afterLength = afterCycle_.length();
        }
        else
        {
            beforeLength = beforePath_.length();
            afterLength = afterPath_.length();
        }
        double maxLength = std::max(beforeLength,afterLength);

        // Compute uniform sampling of key paths.
        // Note: we use a smaller ds for inbetween edges (ds = 2) than for key edges
        // (ds = 5) to reduce flicker caused when resampling beforePath/Cycle and
        // afterPath/Cycle to an equal number of samples.
        double ds = 2.0;
        int minSamples = isClosed() ? 4 : 2;
        int numSamples = std::max(minSamples, (int) (maxLength/ds) + 2);
        QList<EdgeSample> beforeSampling;
        QList<EdgeSample> afterSampling;
        if(isClosed())
        {
            beforeCycle_.sample(numSamples,beforeSampling);
            afterCycle_.sample(numSamples,afterSampling);
        }
        else
        {
            beforePath_.sample(numSamples,beforeSampling);
            afterPath_.sample(numSamples,afterSampling);
        }
        assert(beforeSampling.size() == numSamples);
        assert(afterSampling.size() == numSamples);

        // Interpolate key paths
        double t = time.floatTime(); // in [t1,t2]
        double t1 = beforeTime().floatTime();
        double t2 = afterTime().floatTime();
        double dt = t2-t1;
        double u; // in [0,1]
        if(dt > 0)
            u = (t-t1)/dt;
        else if (t<t1)
            u = 0;
        else
            u = 1;
        QList<EdgeSample> sampling;
        for(int i=0; i<numSamples; ++i)
            sampling << beforeSampling[i] + (afterSampling[i]-beforeSampling[i]) * u;

        // Warp to ensure topological constraints
        if(!isClosed())
        {
            Eigen::Vector2d currentStartPos(sampling.first().x(),sampling.first().y());
            Eigen::Vector2d currentEndPos(sampling.last().x(),sampling.last().y());
            Eigen::Vector2d desiredStartPos = startAnimatedVertex_.pos(time);
            Eigen::Vector2d desiredEndPos = endAnimatedVertex_.pos(time);
            Eigen::Vector2d deltaStartPos =  desiredStartPos - currentStartPos;
            Eigen::Vector2d deltaEndPos =  desiredEndPos - currentEndPos;
            double du = 1.0/(numSamples-1);
            for(int i=0; i<numSamples; ++i)
            {
                double u = i * du;
                Eigen::Vector2d p(sampling[i].x(),sampling[i].y());
                p += (1-u) * deltaStartPos + u*deltaEndPos;
                sampling[i].setX(p[0]);
                sampling[i].setY(p[1]);
            }
        }

        // Do not shrink edge width when edge shrink to vertex
        if(beforePath_.type() == Path::SingleVertex ||
           beforeCycle_.type() == Cycle::SingleVertex)
        {
            for(int i=0; i<numSamples; ++i)
            {
                sampling[i].setWidth(afterSampling[i].width());
            }
        }
        else if (afterPath_.type() == Path::SingleVertex ||
                 afterCycle_.type() == Cycle::SingleVertex)
        {
            for(int i=0; i<numSamples; ++i)
            {
                sampling[i].setWidth(beforeSampling[i].width());
            }
        }

        return sampling;
    }

    void InbetweenEdge::getMesh(
            View3DSettings & viewSettings,
            QList<Eigen::Vector3d>& positions,
            QList<Eigen::Vector3d>& normals,
            QList<int>& indices)
    {
        computeInbetweenSurface(viewSettings);

        // Compute number of quads, early return if no quads
        int m = surf_.size();
        if (m < 2 || norm_.size() != m) {
            return;
        }
        int n = surf_[0].size();
        if (n < 2 || norm_[0].size() != n) {
            return;
        }

        // Append positions and normals
        int p0 = positions.size() + 1; // OBJ indices start at 1
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < n; ++j) {
                positions.append(surf_[i][j]);
                normals.append(norm_[i][j]);
            }
        }

        // Append indices
        for(int i = m-2; i >= 0; i--) {
            int j_ = 0;
            for(int j = 0; j < n;) {
                if (j > 0) {
                    indices.append( i    * n + j_ + p0);
                    indices.append((i+1) * n + j_ + p0);
                    indices.append((i+1) * n + j  + p0);
                    indices.append( i    * n + j  + p0);
                }
                j_ = j;
                if(j == n-1)
                {
                    break;
                }
                else
                {
                    j += cacheK2_;
                    if(j >= n)
                        j = n-1;
                }
            }
            glEnd();
        }
    }

    void InbetweenEdge::triangulate_(Time time, Triangles & out) const
    {
        out.clear();
        if (exists(time))
        {
            QList<EdgeSample> samples = getSampling(time);
            LinearSpline ls(samples);
            if(isClosed())
                ls.makeLoop();
            ls.triangulate(out);
        }
    }

    void InbetweenEdge::triangulate_(double width, Time time, Triangles & out) const
    {
        out.clear();
        if (exists(time))
        {
            QList<EdgeSample> samples = getSampling(time);
            LinearSpline ls(samples);
            if(isClosed())
                ls.makeLoop();
            ls.triangulate(width, out);
        }
    }

    KeyCellSet InbetweenEdge::beforeCells() const
    {
        if(isClosed())
        {
            return beforeCycle_.cells();
        }
        else
        {
            return beforePath_.cells();
        }
    }
    KeyCellSet InbetweenEdge::afterCells() const
    {
        if(isClosed())
        {
            return afterCycle_.cells();
        }
        else
        {
            return afterPath_.cells();
        }
    }

    VertexCellSet InbetweenEdge::startVertices() const
    {
        assert(!isClosed());
        return startAnimatedVertex_.vertices();
    }


    VertexCellSet InbetweenEdge::endVertices() const
    {
        assert(!isClosed());
        return endAnimatedVertex_.vertices();
    }

    Path InbetweenEdge::beforePath() const
    {
        assert(!isClosed());
        return beforePath_;
    }

    Path InbetweenEdge::afterPath() const
    {
        assert(!isClosed());
        return afterPath_;
    }

    AnimatedVertex InbetweenEdge::startAnimatedVertex() const
    {
        assert(!isClosed());
        return startAnimatedVertex_;
    }

    AnimatedVertex InbetweenEdge::endAnimatedVertex() const
    {
        assert(!isClosed());
        return endAnimatedVertex_;

    }

    Cycle InbetweenEdge::beforeCycle() const
    {
        assert(isClosed());
        return beforeCycle_;
    }

    Cycle InbetweenEdge::afterCycle() const
    {
        assert(isClosed());
        return afterCycle_;
    }

    bool InbetweenEdge::check_() const
    {
        // todo
        return true;
    }
}
