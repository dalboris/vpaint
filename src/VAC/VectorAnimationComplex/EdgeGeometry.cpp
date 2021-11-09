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

#include "EdgeGeometry.h"

#include <QTextStream>
#include "../XmlStreamWriter.h"
#include "../XmlStreamReader.h"

#include "../SaveAndLoad.h"
#include "../OpenGL.h"
#include <cmath>
#include "../DevSettings.h"
#include <QtDebug>

using namespace std;

namespace VectorAnimationComplex
{

/***************************************************************
 *                   ABSTRACT EDGE GEOMETRY
 */


EdgeGeometry::EdgeGeometry(double ds) :
    sampling_(),
    isClosed_(false),
    ds_(ds)
{
}

EdgeGeometry::~EdgeGeometry()
{
}

EdgeGeometry * EdgeGeometry::clone()
{
    return new EdgeGeometry();
}

// ---------------------- Save and Load ------------------------


 EdgeGeometry * EdgeGeometry::read(QTextStream & in)
 {
    Field field;
    QString type;
    in >> field >> type;

    if(type == "LinearSpline")
    {
        return new LinearSpline(in);
    }
    // todo: else if, ...
    else
        return 0;
 }

 EdgeGeometry * EdgeGeometry::read(XmlStreamReader & xml)
 {
     // Find curve type and data
     QStringRef str =  xml.attributes().value("curve");
     int i = str.indexOf('(');
     QStringRef curveType = str.left(i);
     QStringRef curveData = str.mid(i+1, str.length()-i-2);

     // Switch on type
     if(curveType == "xywdense")
         return new LinearSpline(curveData);
     else
         return 0;
 }

void EdgeGeometry::save(QTextStream & out)
{
    // Type
    out << Save::newField("Type") << stringType();

    // Overloaded save method
    save_(out);
}

void EdgeGeometry::save_(QTextStream & /*out*/)
{
}


// ---------------------- Drawing ------------------------------

void EdgeGeometry::draw()
{
    // assumes sampling is up to date

    if (sampling_.isEmpty())
        sampling();

    glBegin(GL_LINE_STRIP);
    for(int i=0; i<sampling_.size(); i++)
        glVertex2d(sampling_[i][0], sampling_[i][1]);
    glEnd();
}

void EdgeGeometry::triangulate(Triangles & /*triangles*/)
{
    // TODO
}

void EdgeGeometry::draw(double width)
{
    // assumes sampling is up to date

    if (sampling_.isEmpty())
        sampling();

    glLineWidth(width);
    glBegin(GL_LINE_STRIP);
    for(int i=0; i<sampling_.size(); i++)
        glVertex2d(sampling_[i][0], sampling_[i][1]);
    glEnd();
}

void EdgeGeometry::triangulate(double /*width*/, Triangles & /*triangles*/)
{
    // TODO
}


// --------------- Accessing Curve Geometry --------------------

Eigen::Vector2d EdgeGeometry::pos2d(double s)
{
    EdgeSample sample = pos(s);
    return Eigen::Vector2d(sample.x(),sample.y());
}


EdgeSample EdgeGeometry::pos(double /*s*/) const
{
    return EdgeSample();
}

Eigen::Vector2d EdgeGeometry::der(double /*s*/)
{
    return Eigen::Vector2d(1,0);
}

double EdgeGeometry::length() const
{
    return 0;
}

EdgeSample EdgeGeometry::leftPos() const
{
    return pos(0);
}

EdgeSample EdgeGeometry::rightPos() const
{
    return pos(length());
}

Eigen::Vector2d EdgeGeometry::leftPos2d()
{
    return pos2d(0);
}

Eigen::Vector2d EdgeGeometry::rightPos2d()
{
    return pos2d(length());
}

QList<EdgeSample> EdgeGeometry::edgeSampling() const
{
    // TODO
    return QList<EdgeSample>();
}




// ------------------------ Trimming ---------------------------

EdgeGeometry * EdgeGeometry::trimmed(double /*from*/, double /*to*/)
{
    return new EdgeGeometry();
}



// ----------------------- Sampling ----------------------------

void EdgeGeometry::resample()
{
    resample(ds_);
}

void EdgeGeometry::resample(double ds)
{
    // do nothing if already sampled with the same ds
    if ( !sampling_.isEmpty() && (ds==ds_) )
        return;

    ds_ = ds;
    sampling_.clear();
    resample_(ds);
}

void EdgeGeometry::resample_(double ds)
{
    double L = length();
    if(L>0)
    {
        for(double s=0; s<L; s+=ds)
        {
            EdgeSample sample = pos(s);
            sampling_ << Eigen::Vector2d(sample.x(), sample.y());
        }
        EdgeSample sample = pos(L);
        sampling_ << Eigen::Vector2d(sample.x(), sample.y());
    }
    else
    {
        EdgeSample sample = pos(0);
        sampling_ << Eigen::Vector2d(sample.x(), sample.y());
    }
}

QList<Eigen::Vector2d> & EdgeGeometry::sampling()
{
    if(sampling_.isEmpty())
        resample();
    return sampling_;
}

QList<Eigen::Vector2d> & EdgeGeometry::sampling(double ds)
{
    resample(ds);
    return sampling_;
}
void EdgeGeometry::clearSampling()
{
    sampling_.clear();
}


// --------------------- Manipulating --------------------------

void EdgeGeometry::setLeftRightPos(const Eigen::Vector2d & /*left*/,
                         const Eigen::Vector2d & /*right*/)
{
}

void EdgeGeometry::setRightDer(const Eigen::Vector2d & /*rightDer*/, double /*radius*/, bool /*resample*/)
{
}

void EdgeGeometry::setLeftDer(const Eigen::Vector2d & /*leftDer*/, double /*radius*/, bool /*resample*/)
{
}

void EdgeGeometry::setWidth(double /* newWidth */)
{
}

double EdgeGeometry::updateSculpt(double /*x*/, double /*y*/, double /*radius*/)
{
    return std::numeric_limits<double>::max();
}

void EdgeGeometry::beginSculptDeform(double /*x*/, double /*y*/)
{
}

void EdgeGeometry::continueSculptDeform(double /*x*/, double /*y*/)
{
}

void EdgeGeometry::endSculptDeform()
{
}

EdgeSample EdgeGeometry::sculptVertex() const
{
    return EdgeSample();
}
double EdgeGeometry::arclengthOfSculptVertex() const
{
    return 0;
}

void EdgeGeometry::beginSculptEdgeWidth(double /*x*/, double /*y*/)
{
}

void EdgeGeometry::continueSculptEdgeWidth(double /*x*/, double /*y*/)
{
}

void EdgeGeometry::endSculptEdgeWidth()
{
}

void EdgeGeometry::beginSculptSmooth(double /*x*/, double /*y*/)
{
}

void EdgeGeometry::continueSculptSmooth(double /*x*/, double /*y*/)
{
}

void EdgeGeometry::endSculptSmooth()
{
}

void EdgeGeometry::prepareDragAndDrop()
{
}

void EdgeGeometry::performDragAndDrop(double /*dx*/, double /*dy*/)
{
}

void EdgeGeometry::prepareAffineTransform()
{

}

void EdgeGeometry::performAffineTransform(const Eigen::Affine2d & /*xf*/)
{

}

EdgeGeometry::ClosestVertexInfo EdgeGeometry::closestPoint(double x, double y)
{
    ClosestVertexInfo res;
    res.s = 0;
    res.p = pos(res.s);
    res.d = res.p.distanceTo(EdgeSample(x,y));
    return res;
}
void EdgeGeometry::exportSVG(QTextStream & /*out*/)
{
}
void EdgeGeometry::write(XmlStreamWriter & /*xml*/) const
{
}



/***************************************************************
 *                       LINEAR SPLINE
 */


LinearSpline::LinearSpline(double ds) :
    EdgeGeometry(ds),
    curve_(ds)
{
}

LinearSpline::LinearSpline(const std::vector<EdgeSample,Eigen::aligned_allocator<EdgeSample> > & samples, bool loop)
{
    curve_.setVertices(samples);
    if(loop)
    {
        isClosed_ = true;
        curve_.makeLoop();
    }
    curve_.resample();
}

LinearSpline::LinearSpline(const QList<EdgeSample> & samples, bool loop)
{
    std::vector<EdgeSample,Eigen::aligned_allocator<EdgeSample> > stdvector;
    for (EdgeSample es: samples)
    {
        stdvector.push_back(es);
    }
    curve_.setVertices(stdvector);
    if(loop)
    {
        isClosed_ = true;
        curve_.makeLoop();
    }
    curve_.resample();
}

LinearSpline::LinearSpline(const SculptCurve::Curve<EdgeSample> & other, bool loop) :
    curve_(other)
{
    if(loop)
    {
        isClosed_ = true;
        curve_.makeLoop();
    }
    curve_.resample();
}


LinearSpline::LinearSpline(EdgeGeometry & other)
{
    // get vertices of other geometry
    QList<Eigen::Vector2d> & vertices = other.sampling();

    // create a sampling with default width values
    std::vector<EdgeSample,Eigen::aligned_allocator<EdgeSample> > samples;
    for(int i=0; i<vertices.size(); ++i)
        samples << EdgeSample(vertices[i][0], vertices[i][1]);

    // set the curve to be this sampling
    curve_.setVertices(samples);
    if(other.isClosed())
    {
        isClosed_ = true;
        curve_.makeLoop();
    }
    curve_.resample();
}


LinearSpline::LinearSpline(const QList<Eigen::Vector2d> & vertices, bool loop) //:
    //EdgeGeometry(ds),
    //curve_(ds)
{
    // create a sampling with default width values
    std::vector<EdgeSample,Eigen::aligned_allocator<EdgeSample> > samples;
    for(int i=0; i<vertices.size(); ++i)
        samples << EdgeSample(vertices[i][0], vertices[i][1]);

    // set the curve to be this sampling
    curve_.setVertices(samples);
    if(loop)
    {
        isClosed_ = true;
        curve_.makeLoop();
    }
    curve_.resample();
}

LinearSpline::~LinearSpline()
{
}

LinearSpline * LinearSpline::clone()
{
    return new LinearSpline(curve_, isClosed());
}

// ---------------------- Draw ------------------------

namespace // Anonymous namespace for helper methods
{

// This function assumes that if close = true, then samples.front() ==
// samples.back, that is, that the first/last sample is duplicated.
//
// Terminology:
//
//     * --- * --- *   ...   * --- *
//     S0    S1    S2       Sn-2  Sn-1  : n samples
//
// --> * --> * --> *   ...   * --> * -->
// d0    d1    d2              dn-1  dn  : n+1 left/right tangents
//
//    -->   -->   -->  ...  -->   -->
//    u0    u1    u2        un-2  un-1   : n tangents (=average of left/right d)
//
//    v0    v1    v2              vn-1   : n normal vectors (=perp of u)
//
//     A0    A1    A2       An-2  An-1
//     * --- * --- *   ...   * --- *
//     |     |     |         |     |     = n-1 quads defined by n (A,B) pairs of points
//     * --- * --- *   ...   * --- *
//     B0    B1    B2       Bn-2  Bn-1
//
void triangulateHelper(const QList<EdgeSample> & samples, Triangles & triangles, bool closed = false)
{
    // Initialization and basic case
    triangles.clear();
    int n=samples.size();
    if(n<2)
        return;

    // Helper function
    auto getD = [] (double x1, double y1, double x2, double y2)
    {
        Eigen::Vector2d p1(x1, y1);
        Eigen::Vector2d p2(x2, y2);
        Eigen::Vector2d d = p2-p1; // Assumption: ||d|| > 0. Will result in NaN otherwise
        d.normalize();
        return d;
    };

    // List to store the following:
    //  * n+1 vectors d0, d1, .... , dn
    //  * n   points  A0, A1, .... An-1 and B0, B1, .... Bn-1 (An and Bn are not defined)
    struct QuadInfo    { EIGEN_MAKE_ALIGNED_OPERATOR_NEW
        Eigen::Vector2d d;
        double ax, ay, bx, by; };
    QList<QuadInfo> quads;

    // Computing the di's
    QuadInfo qs;
    if(closed)
        qs.d = getD(samples[n-2].x(), samples[n-2].y(), samples[n-1].x(), samples[n-1].y());
    else
        qs.d = getD(samples[0].x(), samples[0].y(), samples[1].x(), samples[1].y());
    quads << qs;
    for(int i=1; i<n; i++)
    {
        qs.d = getD(samples[i-1].x(), samples[i-1].y(), samples[i].x(), samples[i].y());
        quads << qs;
    }
    if(closed)
        qs.d = getD(samples[0].x(), samples[0].y(), samples[1].x(), samples[1].y());
    else
        qs.d = getD(samples[n-2].x(), samples[n-2].y(), samples[n-1].x(), samples[n-1].y());
    quads << qs;

    // Computing the Ai's and Bi's
    for(int i=0; i<n; i++)
    {
        double h = 0.5 * samples[i].width();

        // Compute bisection basis
        Eigen::Vector2d u = quads[i].d + quads[i+1].d;
        Eigen::Vector2d v;
        double unorm2 = u.squaredNorm();
        if(unorm2 > 0)
        {
            u.normalize();
            v = Eigen::Vector2d(-u[1],u[0]);
        }
        else
        {
            v = quads[i].d;
        }

        quads[i].ax = samples[i].x() + h * v[0];
        quads[i].ay = samples[i].y() + h * v[1];

        quads[i].bx = samples[i].x() - h * v[0];
        quads[i].by = samples[i].y() - h * v[1];
    }

    // tesselate
    for(int i=1; i<n; i++)
    {
        double ax = quads[i-1].ax;
        double ay = quads[i-1].ay;
        double bx = quads[i-1].bx;
        double by = quads[i-1].by;

        double cx = quads[i].ax;
        double cy = quads[i].ay;
        double dx = quads[i].bx;
        double dy = quads[i].by;


        triangles.append(ax,ay,bx,by,dx,dy);
        triangles.append(ax,ay,dx,dy,cx,cy);
    }

    // Start cap
    if (!closed)
    {
        int m = 50;
        double cx = samples.front().x();
        double cy = samples.front().y();
        double r = 0.5 * samples.front().width();
        for(int i=0; i<m; ++i)
        {
            double theta1 = 2 * (double) i * 3.14159 / (double) m ;
            double theta2 = 2 * (double) (i+1) * 3.14159 / (double) m ;

            double ax = cx + r*std::cos(theta1);
            double ay = cy + r*std::sin(theta1);

            double bx = cx + r*std::cos(theta2);
            double by = cy + r*std::sin(theta2);

            triangles.append(ax,ay,bx,by,cx,cy);
        }
    }

    // End cap
    if (!closed)
    {
        int m = 50;
        double cx = samples.back().x();
        double cy = samples.back().y();
        double r = 0.5 * samples.back().width();
        for(int i=0; i<m; ++i)
        {
            double theta1 = 2 * (double) i * 3.14159 / (double) m ;
            double theta2 = 2 * (double) (i+1) * 3.14159 / (double) m ;

            double ax = cx + r*std::cos(theta1);
            double ay = cy + r*std::sin(theta1);

            double bx = cx + r*std::cos(theta2);
            double by = cy + r*std::sin(theta2);

            triangles.append(ax,ay,bx,by,cx,cy);
        }
    }
}
} // End anonymous namespace for helper methods

void LinearSpline::triangulate(Triangles & triangles)
{
    // Don't draw at all too small edges
    // this prevents popping join artefacts during smooth animation
    // when an edge shrinks to a vertex
    if(length() < 0.1)
    {
        triangles.clear();
        return;
    }

    QList<EdgeSample> samples;
    for(int i=0; i<curve_.size(); ++i)
    {
        samples << curve_[i];
    }

    triangulateHelper(samples, triangles, isClosed());
}

void LinearSpline::triangulate(double width, Triangles & triangles)
{
    QList<EdgeSample> samples;
    for(int i=0; i<curve_.size(); ++i)
    {
        EdgeSample sample = curve_[i];
        sample.setWidth(width);
        samples << sample;
    }

    triangulateHelper(samples, triangles, isClosed());
}

void LinearSpline::draw()
{
    Triangles triangles;
    triangulate(triangles);
    triangles.draw();
}

void LinearSpline::draw(double width)
{
    Triangles triangles;
    triangulate(width, triangles);
    triangles.draw();
}


// ---------------------- Save and Load ------------------------

LinearSpline::LinearSpline(QTextStream & in) //:
    //EdgeGeometry(ds),
    //curve_(ds)
{
    Field field;
    QString bracket, nuple;

    // Num Vertices
    int n;
    in >> field >> n;

    // Vertices
    std::vector<EdgeSample,Eigen::aligned_allocator<EdgeSample> > vertices;
    in >> field >> bracket;
    for(int i=0; i<n; i++)
    {
        in >> nuple;
        QStringList list = nuple.split(QRegExp("\\s*[\\(\\,\\)]\\s*"),
                                 QString::SkipEmptyParts);
        vertices << EdgeSample(list[0].toDouble(), list[1].toDouble(), list[2].toDouble());
    }
    in >> bracket;
    curve_.setVertices(vertices);
    clearSampling();
}

void LinearSpline::save_(QTextStream & out)
{
    out << Save::newField("NumVertices") << curve_.size();
    out << Save::newField("Vertices") << "[ ";
    for(int i=0; i<curve_.size(); ++i)
        out << "(" << curve_[i].x() << "," << curve_[i].y() << "," << curve_[i].width() << ") ";
    out << "]";
}

namespace
{
QString double2qstring(double x)
{
    const int numDigits = 15; // decimal representations of double precision floating points
                              // are (typically) meaningless after 15 decimal digits
    // 15 decimal digits guarantees that decimalstring->double->decimalstring is the identity
    // 17 decimal digits guarantees that double->decimalstring->double is the identity

    return QString().setNum(x,'g',numDigits);
}
}

LinearSpline::LinearSpline(const QStringRef & str)
{
    // Clear curve
    curve_.clear();

    // Get data from string
    QStringList strList = str.toString() // Expensive, to change by only using QStringRef
               .split(QRegExp("[\\,\\s]"), QString::SkipEmptyParts); // either ',', or any whitespace character
    QVector<double> d;
    for(int i=0; i<strList.size(); ++i)
        d << strList[i].toDouble();

    // Return if not enough data
    if(d.size() < 1)
        return;

    // Get vertices from data
    std::vector<EdgeSample,Eigen::aligned_allocator<EdgeSample> > vertices;
    int n = (d.size()-1)/3;
    for(int i=0; i<n; i++)
        vertices << EdgeSample(d[3*i+1], d[3*i+2], d[3*i+3]);

    // Set curve
    curve_.setDs(d[0]);
    curve_.setVertices(vertices);
    clearSampling();
}

/*
LinearSpline::LinearSpline(XmlStreamReader & xml)
{
    // Clear curve
    curve_.clear();

    // Get data from string
    QStringList strList =
            xml.attributes().value("curvedata").toString()
               .split(QRegExp("[\\,\\s]"), QString::SkipEmptyParts); // either ',', or any whitespace character
    QVector<double> d;
    for(int i=0; i<strList.size(); ++i)
        d << strList[i].toDouble();

    // Return if not enough data
    if(d.size() < 1)
        return;

    // Get vertices from data
    std::vector<EdgeSample,Eigen::aligned_allocator<EdgeSample> > vertices;
    int n = (d.size()-1)/3;
    for(int i=0; i<n; i++)
        vertices << EdgeSample(d[3*i+1], d[3*i+2], d[3*i+3]);

    // Set curve
    curve_.setDs(d[0]);
    curve_.setVertices(vertices);
    clearSampling();
}
*/

void LinearSpline::write(XmlStreamWriter & xml) const
{
    QString d;
    d += double2qstring(curve_.ds()) + " ";
    const int n = curve_.size();
    for(int i=0; i<n; ++i)
    {
        d += double2qstring(curve_[i].x()) + "," +
             double2qstring(curve_[i].y()) + "," +
             double2qstring(curve_[i].width());

        if(i<n-1) d += " ";
    }

    xml.writeAttribute("curve", "xywdense(" + d + ")");
}


// --------------- Accessing Curve Geometry --------------------

int LinearSpline::size() const { return curve_.size(); }
EdgeSample LinearSpline::operator[] (int i) const { return curve_[i]; }
void LinearSpline::beginSketch(const EdgeSample & sample) { curve_.beginSketch(sample); }
void LinearSpline::continueSketch(const EdgeSample & sample) { curve_.continueSketch(sample); }
void LinearSpline::endSketch() { curve_.endSketch(); }

SculptCurve::Curve<EdgeSample> & LinearSpline::curve()
{
    return curve_;
}

EdgeSample LinearSpline::pos(double s) const
{
    return curve_(s);
}

EdgeSample LinearSpline::leftPos() const
{
    return curve_.start();
}

EdgeSample LinearSpline::rightPos() const
{
    return curve_.end();
}

QList<EdgeSample> LinearSpline::edgeSampling() const
{
    QList<EdgeSample> res;
    for(int i=0; i<curve_.size(); ++i)
        res << curve_[i];
    return res;
}



Eigen::Vector2d LinearSpline::der(double s)
{
    double ds = 1e-3;
    EdgeSample dp = curve_(s+ds) - curve_(s-ds);
    Eigen::Vector2d dpe(dp.x(),dp.y());
    double norm = dpe.norm();

    double tol = 1e-10;
    if(norm<tol)
        return Eigen::Vector2d(1,0);
    return dpe/norm;
}

double LinearSpline::length() const
{
    return curve_.length();
}

EdgeGeometry * LinearSpline::trimmed(double from, double to)
{
    std::vector<double> splitValues;
    splitValues << from << to;
    return new LinearSpline(curve_.split(splitValues)[0]);
}




void LinearSpline::resample_(double ds)
{
    curve_.resample(ds);
    for(int i=0; i<curve_.size(); ++i)
        sampling_ << Eigen::Vector2d(curve_[i].x(), curve_[i].y());
}

void LinearSpline::makeLoop_()
{
    curve_.makeLoop();
}

// --------------------- Manipulating --------------------------

void LinearSpline::setLeftRightPos(const Eigen::Vector2d & left,
                         const Eigen::Vector2d & right)
{
    if(isClosed())
    {
        curve_.resample(true);
    }
    else
    {
        EdgeSample leftSample = curve_.start();
        leftSample.setX(left[0]);
        leftSample.setY(left[1]);

        EdgeSample rightSample = curve_.end();
        rightSample.setX(right[0]);
        rightSample.setY(right[1]);

        curve_.setEndPoints(leftSample, rightSample);
    }
    clearSampling();
}

void LinearSpline::setRightDer(const Eigen::Vector2d & rightDer, double radius, bool resample)
{
    if(radius <= 0)
        return;

    if(radius > length())
        radius = length();

    const bool useRadiusToPreserveTangentEdges = false;
    if(!useRadiusToPreserveTangentEdges)
        radius = length();

    double pi = 3.14159265;

    Eigen::Vector2d oldRightDer = der(length());
    double oldTheta = std::atan2(oldRightDer[1],oldRightDer[0]);
    double newTheta = std::atan2(rightDer[1],rightDer[0]);
    double dtheta = newTheta-oldTheta;
    if(dtheta <= pi)
        dtheta += 2*pi;
    if(dtheta >= pi)
        dtheta -= 2*pi;

    double rightX = curve_.end().x();
    double rightY = curve_.end().y();

    std::vector<EdgeSample,Eigen::aligned_allocator<EdgeSample> > newVertices;
    for(int i=0; i<curve_.size(); ++i)
    {
        // todo: replace by w(distance, radius), where radius is the remaining sculpt radius
        double weightedDtheta = dtheta * curve_.w_( curve_.length() - curve_.arclength(i), radius);
        double c = std::cos(weightedDtheta);
        double s = std::sin(weightedDtheta);

        EdgeSample sample = curve_[i];
        double oldX = sample.x();
        double oldY = sample.y();
        sample.setX( rightX + (oldX-rightX)*c - (oldY-rightY)*s);
        sample.setY( rightY + (oldX-rightX)*s + (oldY-rightY)*c);
        newVertices << sample;
    }

    curve_.setVertices(newVertices);
    if(resample)
        curve_.resample();

    clearSampling();
}

void LinearSpline::setLeftDer(const Eigen::Vector2d & leftDer, double radius, bool resample)
{
    if(radius <= 0)
        return;

    if(radius > length())
        radius = length();

    const bool useRadiusToPreserveTangentEdges = false;
    if(!useRadiusToPreserveTangentEdges)
        radius = length();

    double pi = 3.14159265;

    Eigen::Vector2d oldLeftDer = der(0);
    double oldTheta = std::atan2(oldLeftDer[1],oldLeftDer[0]);
    double newTheta = std::atan2(leftDer[1],leftDer[0]);
    double dtheta = newTheta-oldTheta;
    if(dtheta <= pi)
        dtheta += 2*pi;
    if(dtheta >= pi)
        dtheta -= 2*pi;

    double rightX = curve_.start().x();
    double rightY = curve_.start().y();

    std::vector<EdgeSample,Eigen::aligned_allocator<EdgeSample> > newVertices;
    for(int i=0; i<curve_.size(); ++i)
    {
        // todom replace by w(distance, radius), where radius is the remaining sculpt radius
        double weightedDtheta = dtheta * curve_.w_( curve_.arclength(i), radius);
        double c = std::cos(weightedDtheta);
        double s = std::sin(weightedDtheta);

        EdgeSample sample = curve_[i];
        double oldX = sample.x();
        double oldY = sample.y();
        sample.setX( rightX + (oldX-rightX)*c - (oldY-rightY)*s);
        sample.setY( rightY + (oldX-rightX)*s + (oldY-rightY)*c);
        newVertices << sample;
    }

    curve_.setVertices(newVertices);
    if(resample)
        curve_.resample();

    clearSampling();
}

void LinearSpline::setWidth(double newWidth)
{
    std::vector<EdgeSample,Eigen::aligned_allocator<EdgeSample> > newVertices;
    for(int i=0; i<curve_.size(); ++i)
    {
        EdgeSample sample = curve_[i];
        sample.setWidth(newWidth);
        newVertices << sample;
    }

    curve_.setVertices(newVertices);
}


double LinearSpline::updateSculpt(double x, double y, double radius)
{
    sculptRadius_ = radius;
    return curve_.prepareSculpt(x,y, radius);
}

EdgeSample LinearSpline::sculptVertex() const
{
    return curve_.sculptVertex();
}
double LinearSpline::arclengthOfSculptVertex() const
{
    return curve_.arclengthOfSculptVertex();
}

void LinearSpline::beginSculptDeform(double x, double y)
{
    curve_.beginSculptDeform(x, y);
}

void LinearSpline::continueSculptDeform(double x, double y)
{
    curve_.continueSculptDeform(x, y);
    clearSampling();
}

void LinearSpline::endSculptDeform()
{
    curve_.endSculptDeform();
    clearSampling();
}

double LinearSpline::w_(double s) const // s : arclength distance from sculptVertex to processed vertex
{
    if( s>sculptRadius_ || s<-sculptRadius_)
        return 0;

    double a = (s - sculptRadius_);
    double b = (s + sculptRadius_);
    double sculptRadius2 = sculptRadius_ * sculptRadius_;
    double sculptRadius4 = sculptRadius2 * sculptRadius2;
    return a*a*b*b/sculptRadius4;
}

double LinearSpline::w2_(double d, double r0, double w0) const // cf p 296
{
    if( d>r0 || d<-r0)
        return w0;

    double a = (d - r0);
    double b = (d + r0);
    double radius2 = r0 * r0;
    double radius4 = radius2 * radius2;
    return a*a*b*b/radius4 * (1-w0) + w0;
}

void LinearSpline::beginSculptEdgeWidth(double x, double y)
{
    // save the original geometry
    vertices_.clear();
    arclengths_.clear();
    for(int i=0; i<curve_.size(); ++i)
    {
        vertices_ << curve_[i];
        arclengths_ << curve_.arclength(i);
    }
    sculptIndex_ = curve_.sculptVertexIndex();

    // Store start x and y
    sculptStartX_ = x;
    sculptStartY_ = y;

    // The following list store the vertices to deform
    sculptTemp_.clear();

    // In case nothing to sculpt
    if(sculptIndex_<0 || sculptIndex_> (int) vertices_.size()-1 ) // at the contrary to SculptDeform, being at ends is fine
        return;

    if(isClosed_)
    {
        double l = length();
        double halfLength = 0.5 * l;
        bool handleLargeRadius = false;
        double r0, w0;
        if(sculptRadius_ > halfLength)
        {
            handleLargeRadius = true;
            r0 = halfLength;
            w0 = w_(halfLength);

        }
        for(int i=0; i<size(); ++i)
        {
            // compute signed distance, loop-unaware.
            double d =  arclengths_[sculptIndex_] - arclengths_[i];

            // transform it into unsigned distance, loop-aware: 0 <= d <= length / 2
            if(d>halfLength)
                d -= l;
            if(d<-halfLength)
                d += l;
            if(d<0)
                d *= -1;

            // insert vertex into sculptTemp_
            if(d > sculptRadius_)
                continue;

            double w;
            if(handleLargeRadius)
                w = w2_(d,r0,w0);
            else
                w = w_(d);

            sculptTemp_ << SculptTemp(i, w, vertices_[i].width());
        }
    }
    else
    {
        // add sculpted vertex
        sculptTemp_ << SculptTemp(sculptIndex_, 1, vertices_[sculptIndex_].width());

        // add vertices to sculpt temp before
        int i = sculptIndex_-1;
        while(i>=0)
        {
            double distanceToSculptVertex =  arclengths_[sculptIndex_] - arclengths_[i];
            if(distanceToSculptVertex > sculptRadius_)
                break;

            double w = w_(distanceToSculptVertex);
            sculptTemp_ << SculptTemp(i, w, vertices_[i].width());
            --i;
        }

        // add vertices to sculpt temp after
        i = sculptIndex_+1;
        while(i<size())
        {
            double distanceToSculptVertex =  arclengths_[i] - arclengths_[sculptIndex_];
            if(distanceToSculptVertex > sculptRadius_)
                break;

            double w = w_(distanceToSculptVertex);
            sculptTemp_ << SculptTemp(i, w, vertices_[i].width());
            ++i;
        }
    }
}


void LinearSpline::continueSculptEdgeWidth(double x, double /*y*/)
{
    for(auto & v: sculptTemp_)
    {
        double dSculptWidth = (x - sculptStartX_);
        double newSculptWidth = sculptTemp_[0].width + dSculptWidth;
        if(newSculptWidth<0)
            newSculptWidth *= -1;
        double widthRatio = newSculptWidth / sculptTemp_[0].width;
        vertices_[v.i].setWidth(v.width  * ( 1 + (widthRatio-1) * v.w) );
    }
    curve_.setVertices(vertices_);
    clearSampling();
}

void LinearSpline::endSculptEdgeWidth()
{
    sculptTemp_.clear();
    vertices_.clear();
    arclengths_.clear();
    clearSampling();
}

void LinearSpline::beginSculptSmooth(double /*x*/, double /*y*/)
{
}


void LinearSpline::continueSculptSmooth(double /*x*/, double /*y*/)
{
    curve_.sculptSmooth(0.05);
    clearSampling();
}

void LinearSpline::endSculptSmooth()
{
}

void LinearSpline::prepareDragAndDrop()
{
    dragAndDrop_lastDx_ = 0;
    dragAndDrop_lastDy_ = 0;
}

void LinearSpline::performDragAndDrop(double dx, double dy)
{
    // Safety check
    //if(!isClosed_)
    //    return;

    curve_.translate(dx-dragAndDrop_lastDx_,dy-dragAndDrop_lastDy_);
    dragAndDrop_lastDx_ = dx;
    dragAndDrop_lastDy_ = dy;

    clearSampling();
}

void LinearSpline::prepareAffineTransform()
{
    curveBeforeTransform_ = curve_;
}

void LinearSpline::performAffineTransform(const Eigen::Affine2d & xf)
{
    curve_ = curveBeforeTransform_.transformed(xf);
    clearSampling();
}

EdgeGeometry::ClosestVertexInfo LinearSpline::closestPoint(double x, double y)
{
    // Delegate computation
    SculptCurve::Curve<EdgeSample>::ClosestVertex cv = curve_.findClosestVertex(x,y);

    // Handle result
    if(cv.i == -1)
    {
        // Case where no vertex found
        return EdgeGeometry::closestPoint(x,y);
    }
    else
    {
        ClosestVertexInfo res;
        res.p = curve_[cv.i];
        res.s = curve_.arclength(cv.i);
        res.d = cv.d;
        return res;
    }
}

void LinearSpline::exportSVG(QTextStream & out)
{
    // ---- Compute data to export ----

    std::vector<double> ax, ay, bx, by;

    if(curve_.size() < 2)
        return;

    // helper function
    auto getNormal = [] (double x1, double y1, double x2, double y2)
    {
        Eigen::Vector2d p1(x1, y1);
        Eigen::Vector2d p2(x2, y2);
        Eigen::Vector2d v = p2-p1;
        v.normalize();
        return Eigen::Vector2d(-v[1],v[0]);
    };

    Eigen::Vector2d u = getNormal(curve_[0].x(), curve_[0].y(),
                                  curve_[1].x(), curve_[1].y());
    Eigen::Vector2d p( curve_[0].x(), curve_[0].y() );
    Eigen::Vector2d A = p + curve_[0].width() * 0.5 * u;
    Eigen::Vector2d B = p - curve_[0].width() * 0.5 * u;
    ax.push_back(A[0]);
    ay.push_back(A[1]);
    bx.push_back(B[0]);
    by.push_back(B[1]);
    p = Eigen::Vector2d( curve_[1].x(), curve_[1].y() );
    A = p + curve_[1].width() * 0.5 * u;
    B = p - curve_[1].width() * 0.5 * u;
    ax.push_back(A[0]);
    ay.push_back(A[1]);
    bx.push_back(B[0]);
    by.push_back(B[1]);
    int n = curve_.size();
    if(isClosed()) // clean junction drawing for loops
    {
        n -= 1;
    }
    for(int i=2; i<n; i++)
    {
        Eigen::Vector2d u = getNormal(curve_[i-1].x(), curve_[i-1].y(),
                                      curve_[i].x(), curve_[i].y());
        p = Eigen::Vector2d( curve_[i].x(), curve_[i].y() );
        Eigen::Vector2d A = p + curve_[i].width() * 0.5 * u;
        Eigen::Vector2d B = p - curve_[i].width() * 0.5 * u;
        ax.push_back(A[0]);
        ay.push_back(A[1]);
        bx.push_back(B[0]);
        by.push_back(B[1]);
    }
    if(isClosed()) // clean junction drawing for loops
    {
        ax.push_back(A[0]);
        ay.push_back(A[1]);
        bx.push_back(B[0]);
        by.push_back(B[1]);
    }


    // ---- Write to file ----

    out << "M " << ax[0] << "," << ay[0] << " ";
    for(int i=1; i< (int) ax.size(); ++i)
        out << "L " << ax[i] << "," << ay[i] << " ";
    for(int i = (int)bx.size()-1; i>=0; --i)
        out << "L " << bx[i] << "," << by[i] << " ";
    out << "Z";
}

}
