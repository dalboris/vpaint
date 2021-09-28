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

#ifndef SVGPARSER_H
#define SVGPARSER_H

#include <QColor>
#include "vpaint_global.h"

class QString;
class QXmlStreamAttributes;
struct SvgImportParams;
class XmlStreamReader;

// https://www.w3.org/TR/SVG11/painting.html#SpecifyingPaint
struct Q_VPAINT_EXPORT SvgPaint {
    SvgPaint() : hasColor(false), color(Qt::black) {}
    SvgPaint(const QColor& color) : hasColor(true), color(color) {}
    bool hasColor;
    QColor color;
};

class Q_VPAINT_EXPORT SvgPresentationAttributes
{
public:
    SvgPresentationAttributes();
    void applyChildStyle(const QXmlStreamAttributes& attrs);

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

class Q_VPAINT_EXPORT SvgParser
{
public:
    static void readSvg(XmlStreamReader &xml, const SvgImportParams& params);
};

#endif // SVGPARSER_H
