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
#include <QColor>
#include <QtMath>

#include "VectorAnimationComplex/Cell.h"
#include "VectorAnimationComplex/EdgeSample.h"
#include "XmlStreamReader.h"

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

// https://www.w3.org/TR/SVG11/painting.html#SpecifyingPaint
struct SvgPaint {
    SvgPaint() : hasColor(false), color(Qt::black) {}
    SvgPaint(const QColor& color) : hasColor(true), color(color) {}
    bool hasColor;
    QColor color;
};

class SvgPresentationAttributes
{
public:
    SvgPresentationAttributes();
    void applyChildStyle(XmlStreamReader &xml);

    operator QString() const;

    // Note: fill-opacity, stroke-opacity, and opacity are
    // already factored in the alpha channel of the public
    // variables `fill` and `stroke` below. Also, strokeWidth
    // is set to zero if stroke.hasColor = false.
    SvgPaint fill, stroke;
    double strokeWidth;

private:
    // Computed values after applying inheritance rules.
    //
    // Note that fill-opacity is separately inherited from fill, so we cannot
    // just store fill-opacity inside the alpha value of fill (same for stroke
    // and stroke-opacity).
    //
    SvgPaint fill_, stroke_;
    double fillOpacity_, strokeOpacity_, strokeWidth_;

    // Opacity. This is not inherited but composed as a post-processing step.
    // See comment in the implementation of applyChildStyle(), and:
    // https://www.w3.org/TR/SVG11/masking.html#OpacityProperty
    // https://www.w3.org/TR/SVG11/render.html#Grouping
    double opacity_;

    // Update public variables
    void update_();
};

class SvgParser
{
public:
    SvgParser();

    static void readSvg(XmlStreamReader &xml);

    static SvgPaint parsePaint_(QString s);
    static QColor parseColor_(QString s);

private:
    static bool readRect_(XmlStreamReader &xml, SvgPresentationAttributes &pa);
    static bool readLine_(XmlStreamReader &xml, SvgPresentationAttributes &pa);
    static bool readPolyline_(XmlStreamReader &xml, SvgPresentationAttributes &pa);
    static bool readPolygon_(XmlStreamReader &xml, SvgPresentationAttributes &pa);
    static bool readCircle_(XmlStreamReader &xml, SvgPresentationAttributes &pa);
    static bool readEllipse_(XmlStreamReader &xml, SvgPresentationAttributes &pa);
    static bool readPath_(XmlStreamReader &xml, SvgPresentationAttributes &pa);

    // Utilities
    static QList<PotentialPoint>::iterator populateSamplesRecursive(
            double paramVal, double paramSpan, QList<PotentialPoint> & edgeSamples,
            QList<PotentialPoint>::iterator pointLoc, double strokeWidth, double ds,
            std::function<Eigen::Vector2d (double)> getPoint);

};

#endif // SVGPARSER_H
