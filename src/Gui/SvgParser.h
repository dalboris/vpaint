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

class QString;

class SvgParser
{
public:
    SvgParser();
//private:
    QColor parseColor_(QString s);
    bool readRect_(XmlStreamReader & xml);
    bool readLine_(XmlStreamReader & xml);
    bool readPolyline_(XmlStreamReader & xml);
    bool readPolygon_(XmlStreamReader & xml);
    bool readCircle_(XmlStreamReader & xml);
    void readSvg_(XmlStreamReader & xml);
};

class SvgPresentationAttributes
{
public:
    SvgPresentationAttributes(XmlStreamReader & xml, SvgParser & parser);
    QColor fill, stroke;
    qreal strokeWidth;
};

#endif // SVGPARSER_H
