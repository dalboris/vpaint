// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef SVGPARSER_H
#define SVGPARSER_H

#include <QColor>

class QString;
class XmlStreamReader;

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
    static void readSvg(XmlStreamReader &xml);
};

#endif // SVGPARSER_H
