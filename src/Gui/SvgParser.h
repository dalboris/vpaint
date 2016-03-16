// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef SVGPARSER_H
#define SVGPARSER_H

#include <QtGlobal>
#include <XmlStreamReader.h>
#include <QColor>
#include <QtMath>

#include <VectorAnimationComplex/EdgeSample.h>

class QString;

class PotentialPoint
{
public:
    PotentialPoint(double x, double y, double width) : sample_(x, y, width), left_(-1), right_(-1) {}
    PotentialPoint(Eigen::Vector2d point, double width) : sample_(point[0], point[1], width), left_(-1), right_(-1) {}
    PotentialPoint(Eigen::Vector3d point) : sample_(point), left_(-1), right_(-1) {}
    PotentialPoint(VectorAnimationComplex::EdgeSample sample) : sample_(sample), left_(-1), right_(-1) {}

    double getLeftTangent() const { return left_; }
    double getRightTangent() const { return right_; }
    double getX() const { return sample_.x(); }
    double getY() const { return sample_.y(); }
    double getWidth() const { return sample_.width(); }
    double distanceTo(const PotentialPoint & other) const { return sample_.distanceTo(other.getEdgeSample()); }
    VectorAnimationComplex::EdgeSample getEdgeSample() const { return sample_; }

    void setLeftTangent(double angle) { left_ = fmod((fmod(angle, (2 * M_PI)) + (2 * M_PI)), (2 * M_PI)); }
    void setRightTangent(double angle) { right_ = fmod((fmod(angle, (2 * M_PI)) + (2 * M_PI)), (2 * M_PI)); }

    bool isSmooth() { return left_ > 0 && right_ > 0 && qAbs(left_ - right_) < angleThreshold; }

private:
    VectorAnimationComplex::EdgeSample sample_;
    static constexpr double angleThreshold = 0.01 * M_PI;
    double left_, right_;
};

class SvgParser;

class SvgPresentationAttributes
{
public:
    SvgPresentationAttributes(XmlStreamReader & xml, SvgParser & parser);
    QColor fill, stroke;
    qreal strokeWidth;

    bool hasFill() const { return fill.isValid(); }
};

class SvgParser
{
public:
    SvgParser();
//private:
    bool readRect_(XmlStreamReader & xml);
    bool readLine_(XmlStreamReader & xml);
    bool readPolyline_(XmlStreamReader & xml);
    bool readPolygon_(XmlStreamReader & xml);
    bool readCircle_(XmlStreamReader & xml);
    bool readEllipse_(XmlStreamReader & xml);
    bool readPath_(XmlStreamReader & xml);
    void readSvg_(XmlStreamReader & xml);

    // Utilities
    QColor parseColor_(QString s);
    double getNextDouble(QString & source, bool * ok = 0);
    Eigen::Vector2d getNextCoordinatePair(QString & source, bool * ok = 0);
    void trimFront(QString & string, QVector<QChar> chars = { 0x20, 0x9, 0xD, 0xA });
    void populateSamplesRecursive(double paramVal, double paramSpan, QList<PotentialPoint> & edgeSamples, QList<PotentialPoint>::iterator pointLoc, double strokeWidth, double ds, std::function<Eigen::Vector2d (double)> getPoint);

    // Path things
    bool parsePath(QString & data, const SvgPresentationAttributes & pa, const Eigen::Vector2d startPos = Eigen::Vector2d(0, 0));
    bool addLineTo(QList<PotentialPoint> & samplingPoints, QString & data, const SvgPresentationAttributes & pa, bool relative);
    bool addVerticalLineTo(QList<PotentialPoint> & samplingPoints, QString & data, const SvgPresentationAttributes & pa, bool relative);
    bool addHorizontalLineTo(QList<PotentialPoint> & samplingPoints, QString & data, const SvgPresentationAttributes & pa, bool relative);
    bool addCurveTo(QList<PotentialPoint> & samplingPoints, QString & data, bool relative);
    bool addSmoothCurveTo(QList<PotentialPoint> & samplingPoints, QString & data, bool relative);
    bool addQuadraticBezierCurveTo(QList<PotentialPoint> & samplingPoints, QString & data, bool relative);
    bool addSmoothQuadraticBezierCurveTo(QList<PotentialPoint> & samplingPoints, QString & data, bool relative);
    bool addEllipticalArc(QList<PotentialPoint> & samplingPoints, QString & data, bool relative);
    Eigen::Vector2d finishPath(QList<PotentialPoint> & samplingPoints, const SvgPresentationAttributes pa, bool closed = false);
};

#endif // SVGPARSER_H
