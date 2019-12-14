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

#ifndef VAC_EDGE_GEOMETRY_H
#define VAC_EDGE_GEOMETRY_H

#include <QList>
#include <QString>
#include "Eigen.h"

#include "EdgeSample.h"
#include "SculptCurve.h"
#include "Triangles.h"

class QTextStream;
class XmlStreamWriter;
class XmlStreamReader;

namespace VectorAnimationComplex
{

class EdgeGeometry
{
public:
    EdgeGeometry(double ds = 5.0);
    virtual ~EdgeGeometry();


    virtual EdgeGeometry * clone();

    // draw the edge with its stored variable width
    virtual void draw();
    virtual void triangulate(Triangles & triangles);

    // draw the edges with a fixed width (ignore stored width)
    virtual void draw(double width);
    virtual void triangulate(double width, Triangles & triangles);

    // override these for your specific curve representation
    Eigen::Vector2d pos2d(double s);
    virtual EdgeSample pos(double s) const;
    virtual Eigen::Vector2d der(double s);
    virtual double length() const;
    virtual EdgeGeometry * trimmed(double from, double to);

    // convenient methods
    virtual EdgeSample leftPos() const;
    virtual EdgeSample rightPos() const;
    Eigen::Vector2d leftPos2d();
    Eigen::Vector2d rightPos2d();

    // no  need  to   override  sample(ds)  unless  pos(s)  is
    // expensive and  sample(ds) can be  done more efficiently
    // than in length/ds * pos(s) operations.
    void resample();
    void resample(double ds);
    QList<Eigen::Vector2d> & sampling();
    QList<Eigen::Vector2d> & sampling(double ds);
    virtual QList<EdgeSample> edgeSampling() const;

    void clearSampling(); // call this if the geometry changed

    // geometry manipulation
    virtual void setLeftRightPos(const Eigen::Vector2d & left,
                         const Eigen::Vector2d & right);
    virtual void setRightDer(const Eigen::Vector2d & rightDer, double radius, bool resample);
    virtual void setLeftDer(const Eigen::Vector2d & leftDer, double radius, bool resample);
    virtual void setWidth(double newWidth);
    // sculpting
    virtual double updateSculpt(double x, double y, double radius);
    virtual EdgeSample sculptVertex() const;
    virtual double arclengthOfSculptVertex() const;
    // deform
    virtual void beginSculptDeform(double x, double y);
    virtual void continueSculptDeform(double x, double y);
    virtual void endSculptDeform();
    // change edge width
    virtual void beginSculptEdgeWidth(double x, double y);
    virtual void continueSculptEdgeWidth(double x, double y);
    virtual void endSculptEdgeWidth();
    // smooth
    virtual void beginSculptSmooth(double x, double y);
    virtual void continueSculptSmooth(double x, double y);
    virtual void endSculptSmooth();
    // loop drag and drop
    virtual void prepareDragAndDrop();
    virtual void performDragAndDrop(double dx, double dy);
    // affine transform
    virtual void prepareAffineTransform();
    virtual void performAffineTransform(const Eigen::Affine2d & xf);


    // Save and Load
    static EdgeGeometry * read(QTextStream & in);
    static EdgeGeometry * read(XmlStreamReader & xml);
    void save(QTextStream & out);
    virtual void exportSVG(QTextStream & out);
    virtual QString stringType() const {return "EdgeGeometry";}
    virtual void write(XmlStreamWriter & xml) const;

    // Others
    bool isClosed() const {return isClosed_;}
    void makeLoop() { isClosed_ = true; makeLoop_(); }

    // Closest vertex queries
    struct ClosestVertexInfo {
        EdgeSample p; // closest point
        double s;     // arclength of the closest point
        double d;      // distance from the query to the closest point
    };
    virtual ClosestVertexInfo closestPoint(double x, double y);

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

protected:
    // override this  only if sample_(ds) can be  done in less
    // than length/ds * pos(s) operations.
    virtual void resample_(double ds);
    QList<Eigen::Vector2d> sampling_;

    // Save and Load
    virtual void save_(QTextStream & out);

    // Others
    virtual void makeLoop_() {}
    bool isClosed_;


private:
    double ds_;

};

class LinearSpline: public EdgeGeometry
{
public:
    LinearSpline(double ds = 5.0);
    LinearSpline(const QList<EdgeSample> & samples, bool loop = false);
    LinearSpline(const std::vector<EdgeSample,Eigen::aligned_allocator<EdgeSample> > & samples, bool loop = false);
    LinearSpline(const SculptCurve::Curve<EdgeSample> & other, bool loop = false);
    LinearSpline(EdgeGeometry & other); // non-const cause
                            // sampling computed
    LinearSpline(const QList<Eigen::Vector2d> & vertices, bool loop = false);
    virtual ~LinearSpline();

    LinearSpline * clone();

    virtual void draw();
    virtual void draw(double width);
    virtual void triangulate(Triangles & triangles);
    virtual void triangulate(double width, Triangles & triangles);

    void exportSVG(QTextStream & out);

    virtual EdgeSample leftPos() const;
    virtual EdgeSample rightPos() const;
    virtual QList<EdgeSample> edgeSampling() const;

    EdgeSample pos(double s) const;
    Eigen::Vector2d der(double s);
    double length() const;
    EdgeGeometry * trimmed(double from, double to);
    void setLeftRightPos(const Eigen::Vector2d & left,
                   const Eigen::Vector2d & right);
    void setRightDer(const Eigen::Vector2d & rightDer, double radius, bool resample);
    void setLeftDer(const Eigen::Vector2d & leftDer, double radius, bool resample);
    void setWidth(double newWidth);

    // Sculpting
    double updateSculpt(double x, double y, double radius);
    EdgeSample sculptVertex() const;
    double arclengthOfSculptVertex() const;
    // Deform
    void beginSculptDeform(double x, double y);
    void continueSculptDeform(double x, double y);
    void endSculptDeform();
    // Change Edge Width
    void beginSculptEdgeWidth(double x, double y);
    void continueSculptEdgeWidth(double x, double y);
    void endSculptEdgeWidth();
    // Change Edge Width
    void beginSculptSmooth(double x, double y);
    void continueSculptSmooth(double x, double y);
    void endSculptSmooth();
    // loop drag and drop
    void prepareDragAndDrop();
    void performDragAndDrop(double dx, double dy);
    // affine transform
    void prepareAffineTransform();
    void performAffineTransform(const Eigen::Affine2d & xf);
    // Compute closest point on curve
    ClosestVertexInfo closestPoint(double x, double y);


    LinearSpline(QTextStream & in);
    //LinearSpline(XmlStreamReader & xml);
    LinearSpline(const QStringRef & str); // str = curve data from XML, without the type
    QString stringType() const {return "LinearSpline";}

    SculptCurve::Curve<EdgeSample> & curve();

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    // Sketch
    int size() const;
    EdgeSample operator[] (int i) const;
    void beginSketch(const EdgeSample & resample);
    void continueSketch(const EdgeSample & resample);
    void endSketch();

protected:
    void save_(QTextStream & out);
    void write(XmlStreamWriter & xml) const;

private:
    void resample_(double ds); // linear time
    //void computeLength();
    //double length_;
    //QList<Eigen::Vector2d> vertices_;

    SculptCurve::Curve<EdgeSample> curve_;

    // Store initial curve for affine tranform
    SculptCurve::Curve<EdgeSample> curveBeforeTransform_;

    // Others
    void makeLoop_();

    // sculpting (needs cleaning...)
    double sculptRadius_;
    double w_(double s) const;
    double w2_(double d, double r0, double w0) const;
    std::vector<EdgeSample,Eigen::aligned_allocator<EdgeSample> > vertices_;
    std::vector<double> arclengths_;
    int sculptIndex_;
    double sculptStartX_;
    double sculptStartY_;
    struct SculptTemp
    {
        SculptTemp(int i, double w, double width) : i(i), w(w), width(width) {}
        int i; double w; double width;
    };
    std::vector<SculptTemp> sculptTemp_;

    // drag and drop
    double dragAndDrop_lastDx_;
    double dragAndDrop_lastDy_;
};

}

#endif
