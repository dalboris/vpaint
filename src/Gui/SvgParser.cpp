// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "SvgParser.h"
#include "XmlStreamReader.h"

#include <QString>
#include <QVector>
#include <QDebug>
#include <QStringRef>
#include <VectorAnimationComplex/VAC.h>
#include <VectorAnimationComplex/KeyVertex.h>
#include <VectorAnimationComplex/KeyEdge.h>
#include <VectorAnimationComplex/KeyFace.h>
#include <VectorAnimationComplex/EdgeSample.h>
#include <VectorAnimationComplex/SculptCurve.h>
#include <VectorAnimationComplex/EdgeGeometry.h>
#include <View.h>
#include <Global.h>
#include <QRegExp>

SvgParser::SvgParser()
{

}

// Parses color from string, will probably be moved to a class like CSSColor
// This implements the most of the W3 specifications
// found at https://www.w3.org/TR/SVG11/types.html#DataTypeColor
// It also extends the specifications in a few minor ways
// This includes more flexible whitespace and some CSS3 features (hsl)
QColor SvgParser::parseColor_(QString s)
{
    // Remove excess whitespace
    s = s.trimmed();
    if(s == "none") {
        return QColor();
    }
    if(s.startsWith("rgba") && s.endsWith(")") && s.contains("("))
    {
        // Remove rgba()
        s.remove(0, 4).chop(1);
        s = s.trimmed().remove(0, 1);

        // Split into elements with comma separating them
        QStringList sSplit = s.split(',');

        // If it doesn't have exactly four elements, return an invalid color
        if(sSplit.size() != 4)
        {
            return QColor();
        }

        int colors[3];

        // Handle rgb channels
        for(int i = 0; i < 3; i++)
        {
            QString element(sSplit[i]);

            // More trimming
            element = element.trimmed();

            // Determine if it is *% or * format
            if(element.endsWith("%"))
            {
                // Remove % sign
                element.chop(1);

                colors[i] = qRound(qBound(0.0, element.toDouble(), 100.0) * 2.55);
            }
            else
            {
                colors[i] = qBound(0, qRound(element.toDouble()), 255);
            }
        }
        // Alpha channel is a double from 0.0 to 1.0 inclusive
        double alpha = qBound(0.0, sSplit[3].toDouble(), 1.0);

        // Return result
        QColor color = QColor(colors[0], colors[1], colors[2]);
        color.setAlphaF(alpha);
        return color;
    }
    else if(s.startsWith("rgb") && s.endsWith(")") && s.contains("("))
    {
        // Remove rgb()
        s.remove(0, 3).chop(1);
        s = s.trimmed().remove(0, 1);

        // Split into elements with comma separating them
        QStringList sSplit = s.split(',');

        // If it doesn't have exactly three elements, return an invalid color
        if(sSplit.size() != 3)
        {
            return QColor();
        }

        int colors[3];

        for(int i = 0; i < 3; i++)
        {
            QString element(sSplit[i]);

            // More trimming
            s = s.trimmed();

            // Determine if it is *% or * format
            if(element.endsWith("%"))
            {
                // Remove % sign
                element.chop(1);

                // Convert to number and add element to list (100% -> 255)
                colors[i] = qRound(qBound(0.0, element.toDouble(), 100.0) * 2.55);
            }
            else
            {
                colors[i] = qBound(0, qRound(element.toDouble()), 255);
            }
        }

        // Return result
        return QColor(colors[0], colors[1], colors[2]);
    }
    else if(s.startsWith("hsla") && s.endsWith(")") && s.contains("("))
    {
        // Remove hsla()
        s.remove(0, 4).chop(1);
        s = s.trimmed().remove(0, 1);

        // Split into elements with comma separating them
        QStringList sSplit = s.split(',');

        // If it doesn't have exactly three elements, return an invalid color
        // If saturation and lightness are not percentages, also return invalid
        if(sSplit.size() != 4 || !sSplit[1].endsWith("%") || !sSplit[2].endsWith("%"))
        {
            return QColor();
        }

        // Hue is an angle from 0-359 inclusive
        int hue = qRound(sSplit[0].toDouble());
        // As an angle, hue loops around
        hue = ((hue % 360) + 360) % 360;

        // Saturation and lightness are read as percentages and mapped to the range 0-255
        // Remove percentage signs
        sSplit[1].chop(1);
        sSplit[2].chop(1);
        int saturation = qRound(qBound(0.0, sSplit[1].toDouble(), 100.0) * 2.55);
        int lightness = qRound(qBound(0.0, sSplit[2].toDouble(), 100.0) * 2.55);

        // Alpha channel is a double from 0.0 to 1.0 inclusive
        double alpha = qBound(0.0, sSplit[3].toDouble(), 1.0);

        // Return result
        QColor color = QColor();
        color.setHsl(hue, saturation, lightness);
        color.setAlphaF(alpha);
        return color;
    }
    else if(s.startsWith("hsl") && s.endsWith(")") && s.contains("("))
    {
        // Remove hsl()
        s.remove(0, 3).chop(1);
        s = s.trimmed().remove(0, 1);

        // Split into elements with comma separating them
        QStringList sSplit = s.split(',');

        // If it doesn't have exactly three elements, return an invalid color
        // If saturation and lightness are not percentages, also return invalid
        if(sSplit.size() != 3 || !sSplit[1].endsWith("%") || !sSplit[2].endsWith("%"))
        {
            return QColor();
        }

        // Hue is an angle from 0-359 inclusive
        int hue = qRound(sSplit[0].toDouble());
        // As an angle, hue loops around
        hue = ((hue % 360) + 360) % 360;

        // Saturation and lightness are read as percentages and mapped to the range 0-255
        // Remove percentage signs
        sSplit[1].chop(1);
        sSplit[2].chop(1);
        int saturation = qRound(qBound(0.0, sSplit[1].toDouble(), 100.0) * 2.55);
        int lightness = qRound(qBound(0.0, sSplit[2].toDouble(), 100.0) * 2.55);

        // Return result
        QColor color = QColor();
        color.setHsl(hue, saturation, lightness);
        return color;
    }
    else
    {
        // This handles named constants and #* formats
        return QColor(s);
    }
}

// Reads a <rect> object
// https://www.w3.org/TR/SVG11/shapes.html#RectElement
// @return true on success, false on failure
bool SvgParser::readRect_(XmlStreamReader & xml)
{
    // Check to make sure we are reading a rect object
    if(xml.name() != "rect") return true;

    bool okay;

    // Get attributes

    // X position
    double x = xml.attributes().hasAttribute("x") ? xml.attributes().value("x").toDouble(&okay) : 0;
    if(!okay) x = 0;

    // Y position
    double y = xml.attributes().hasAttribute("y") ? xml.attributes().value("y").toDouble(&okay) : 0;
    if(!okay) y = 0;

    // Width
    double width = xml.attributes().value("width").toDouble(&okay);
    // Error, width isn't a real number
    if(!okay) return false;

    // Height
    double height = xml.attributes().value("height").toDouble(&okay);
    // Error, height isn't a real number
    if(!okay) return false;

    // Negative width or height results in an error
    if(width < 0 || height < 0) return false;

    // A width or height of 0 does not result in an error, but disables rendering of the object
    if(width == 0 || height == 0) return true;

    // The rx and ry attributes have a slightly more advanced default value, see W3 specifications for details
    double rx, ry;
    bool rxOkay = false, ryOkay = false;
    if(xml.attributes().hasAttribute("rx"))
    {
        rx = xml.attributes().value("rx").toDouble(&rxOkay);
    }
    if(xml.attributes().hasAttribute("ry"))
    {
        ry = xml.attributes().value("ry").toDouble(&ryOkay);
    }
    if(!rxOkay && !ryOkay)
    {
        rx = ry = 0;
    }
    else if(rxOkay && !ryOkay)
    {
        ry = rx;
    }
    else if(!rxOkay && ryOkay)
    {
        rx = ry;
    }
    rx = qBound(0.0, rx, width / 2);
    ry = qBound(0.0, ry, height / 2);

    // Get presentation attributes
    SvgPresentationAttributes pa(xml, *this);

    // Build verticies and edges
    VectorAnimationComplex::KeyVertex * v1 = global()->currentVAC()->newKeyVertex(global()->activeTime(), Eigen::Vector2d(x, y));
    VectorAnimationComplex::KeyVertex * v2 = global()->currentVAC()->newKeyVertex(global()->activeTime(), Eigen::Vector2d(x + width, y));
    VectorAnimationComplex::KeyVertex * v3 = global()->currentVAC()->newKeyVertex(global()->activeTime(), Eigen::Vector2d(x + width, y + height));
    VectorAnimationComplex::KeyVertex * v4 = global()->currentVAC()->newKeyVertex(global()->activeTime(), Eigen::Vector2d(x, y + height));
    SculptCurve::Curve<VectorAnimationComplex::EdgeSample> c1(VectorAnimationComplex::EdgeSample(v1->pos()[0], v1->pos()[1], pa.strokeWidth), VectorAnimationComplex::EdgeSample(v2->pos()[0], v2->pos()[1], pa.strokeWidth));
    SculptCurve::Curve<VectorAnimationComplex::EdgeSample> c2(VectorAnimationComplex::EdgeSample(v2->pos()[0], v2->pos()[1], pa.strokeWidth), VectorAnimationComplex::EdgeSample(v3->pos()[0], v3->pos()[1], pa.strokeWidth));
    SculptCurve::Curve<VectorAnimationComplex::EdgeSample> c3(VectorAnimationComplex::EdgeSample(v3->pos()[0], v3->pos()[1], pa.strokeWidth), VectorAnimationComplex::EdgeSample(v4->pos()[0], v4->pos()[1], pa.strokeWidth));
    SculptCurve::Curve<VectorAnimationComplex::EdgeSample> c4(VectorAnimationComplex::EdgeSample(v4->pos()[0], v4->pos()[1], pa.strokeWidth), VectorAnimationComplex::EdgeSample(v1->pos()[0], v1->pos()[1], pa.strokeWidth));
    VectorAnimationComplex::KeyEdge * e1 = global()->currentVAC()->newKeyEdge(global()->activeTime(), v1, v2, (new VectorAnimationComplex::LinearSpline(c1)), pa.strokeWidth);
    VectorAnimationComplex::KeyEdge * e2 = global()->currentVAC()->newKeyEdge(global()->activeTime(), v2, v3, (new VectorAnimationComplex::LinearSpline(c2)), pa.strokeWidth);
    VectorAnimationComplex::KeyEdge * e3 = global()->currentVAC()->newKeyEdge(global()->activeTime(), v3, v4, (new VectorAnimationComplex::LinearSpline(c3)), pa.strokeWidth);
    VectorAnimationComplex::KeyEdge * e4 = global()->currentVAC()->newKeyEdge(global()->activeTime(), v4, v1, (new VectorAnimationComplex::LinearSpline(c4)), pa.strokeWidth);

    // Apply stroke color
    v1->setColor(pa.stroke);
    v2->setColor(pa.stroke);
    v3->setColor(pa.stroke);
    v4->setColor(pa.stroke);
    e1->setColor(pa.stroke);
    e2->setColor(pa.stroke);
    e3->setColor(pa.stroke);
    e4->setColor(pa.stroke);

    // Add fill
    if(xml.attributes().value("fill").trimmed() != "none")
    {
        QList<VectorAnimationComplex::KeyHalfedge> edges;
        edges.append(VectorAnimationComplex::KeyHalfedge(e1, true));
        edges.append(VectorAnimationComplex::KeyHalfedge(e2, true));
        edges.append(VectorAnimationComplex::KeyHalfedge(e3, true));
        edges.append(VectorAnimationComplex::KeyHalfedge(e4, true));
        VectorAnimationComplex::Cycle cycle(edges);
        VectorAnimationComplex::KeyFace * face = global()->currentVAC()->newKeyFace(cycle);
        face->setColor(pa.fill);
    }

    return true;
}

bool SvgParser::readLine_(XmlStreamReader & xml)
{
    // Check to make sure we are reading a line object
    if(xml.name() != "line") return true;

    bool okay;

    // Get attributes

    // X position 1
    double x1 = xml.attributes().hasAttribute("x1") ? xml.attributes().value("x1").toDouble(&okay) : 0;
    if(!okay) x1 = 0;

    // Y position 1
    double y1 = xml.attributes().hasAttribute("y1") ? xml.attributes().value("y1").toDouble(&okay) : 0;
    if(!okay) y1 = 0;

    // X position 2
    double x2 = xml.attributes().hasAttribute("x2") ? xml.attributes().value("x2").toDouble(&okay) : 0;
    if(!okay) x2 = 0;

    // Y position 2
    double y2 = xml.attributes().hasAttribute("y2") ? xml.attributes().value("y2").toDouble(&okay) : 0;
    if(!okay) y2 = 0;

    // Get presentation attributes
    SvgPresentationAttributes pa(xml, *this);

    // Build verticies and edges
    VectorAnimationComplex::KeyVertex * v1 = global()->currentVAC()->newKeyVertex(global()->activeTime(), Eigen::Vector2d(x1, y1));
    VectorAnimationComplex::KeyVertex * v2 = global()->currentVAC()->newKeyVertex(global()->activeTime(), Eigen::Vector2d(x2, y2));
    SculptCurve::Curve<VectorAnimationComplex::EdgeSample> c(VectorAnimationComplex::EdgeSample(v1->pos()[0], v1->pos()[1], pa.strokeWidth), VectorAnimationComplex::EdgeSample(v2->pos()[0], v2->pos()[1], pa.strokeWidth));
    VectorAnimationComplex::KeyEdge * e = global()->currentVAC()->newKeyEdge(global()->activeTime(), v1, v2, (new VectorAnimationComplex::LinearSpline(c)), pa.strokeWidth);

    // Apply stroke color
    v1->setColor(pa.stroke);
    v2->setColor(pa.stroke);
    e->setColor(pa.stroke);

    return true;
}

bool SvgParser::readPolyline_(XmlStreamReader &xml)
{
    // Check to make sure we are reading a polyline object
    if(xml.name() != "polyline" || !xml.attributes().hasAttribute("points")) return true;

    bool okay;

    // Get presentation attributes
    SvgPresentationAttributes pa(xml, *this);

    // Read and split points
    // Technically the parsing of separators is a bit more complicated,
    // but this will suffice as it correctly handles all standard-conforming svgs
    QStringList points = xml.attributes().value("points").toString().split(QRegExp("[\\s,]+"), QString::SkipEmptyParts);

    // Don't render isn't at least one complete coordinate
    if(points.size() < 2) return true;

    QVector<VectorAnimationComplex::KeyVertex *> verticies(points.size() / 2);

    // Parse points
    for(int i = 0; i < verticies.size(); i++) {
        // X
        double x = points[i * 2].toDouble(&okay);
        if(!okay) return false;

        // Y
        double y = points[i * 2 + 1].toDouble(&okay);
        if(!okay) return false;

        verticies[i] = global()->currentVAC()->newKeyVertex(global()->activeTime(), Eigen::Vector2d(x, y));
        verticies[i]->setColor(pa.stroke);
    }

    // Create edges
    for(int i = 1; i < verticies.size(); i++) {
        VectorAnimationComplex::KeyEdge * e = global()->currentVAC()->newKeyEdge(global()->activeTime(), verticies[i-1], verticies[i], (new VectorAnimationComplex::LinearSpline(SculptCurve::Curve<VectorAnimationComplex::EdgeSample>(VectorAnimationComplex::EdgeSample(verticies[i-1]->pos()[0], verticies[i-1]->pos()[1], pa.strokeWidth), VectorAnimationComplex::EdgeSample(verticies[i]->pos()[0], verticies[i]->pos()[1], pa.strokeWidth)))), pa.strokeWidth);
        e->setColor(pa.stroke);
    }

    return true;
}

bool SvgParser::readPolygon_(XmlStreamReader &xml)
{
    // Check to make sure we are reading a polygon object
    if(xml.name() != "polygon" || !xml.attributes().hasAttribute("points")) return true;

    bool okay;

    // Get presentation attributes
    SvgPresentationAttributes pa(xml, *this);

    // Read and split points
    // Technically the parsing of separators is a bit more complicated,
    // but this will suffice as it correctly handles all standard-conforming svgs
    QStringList points = xml.attributes().value("points").toString().split(QRegExp("[\\s,]+"), QString::SkipEmptyParts);

    // Fail if there isn't at least one complete coordinate
    if(points.size() < 2) return false;

    QVector<VectorAnimationComplex::KeyVertex *> verticies(points.size() / 2);

    // Parse points
    for(int i = 0; i < verticies.size(); i++) {
        // X
        double x = points[i * 2].toDouble(&okay);
        if(!okay) return false;

        // Y
        double y = points[i * 2 + 1].toDouble(&okay);
        if(!okay) return false;

        verticies[i] = global()->currentVAC()->newKeyVertex(global()->activeTime(), Eigen::Vector2d(x, y));
        verticies[i]->setColor(pa.stroke);
    }

    // Create Edges
    QVector<VectorAnimationComplex::KeyEdge *> edges(verticies.size() - 1);
    for(int i = 1; i < verticies.size(); i++) {
        VectorAnimationComplex::KeyEdge * e = global()->currentVAC()->newKeyEdge(global()->activeTime(), verticies[i-1], verticies[i], (new VectorAnimationComplex::LinearSpline(SculptCurve::Curve<VectorAnimationComplex::EdgeSample>(VectorAnimationComplex::EdgeSample(verticies[i-1]->pos()[0], verticies[i-1]->pos()[1], pa.strokeWidth), VectorAnimationComplex::EdgeSample(verticies[i]->pos()[0], verticies[i]->pos()[1], pa.strokeWidth)))), pa.strokeWidth);
        e->setColor(pa.stroke);
        edges[i-1] = e;
    }

    // Close the loop if it isn't yet closed
    if(verticies.first()->pos() != verticies.last()->pos()) {
        VectorAnimationComplex::KeyEdge * e = global()->currentVAC()->newKeyEdge(global()->activeTime(), verticies.last(), verticies[0], (new VectorAnimationComplex::LinearSpline(SculptCurve::Curve<VectorAnimationComplex::EdgeSample>(VectorAnimationComplex::EdgeSample(verticies.last()->pos()[0], verticies.last()->pos()[1], pa.strokeWidth), VectorAnimationComplex::EdgeSample(verticies[0]->pos()[0], verticies[0]->pos()[1], pa.strokeWidth)))), pa.strokeWidth);
        e->setColor(pa.stroke);
        edges.push_back(e);
    }

    // Add fill
    if(xml.attributes().value("fill").trimmed() != "none")
    {
        QList<VectorAnimationComplex::KeyHalfedge> halfEdges;
        for(int i = 0; i < edges.size(); i++) {
            halfEdges.append(VectorAnimationComplex::KeyHalfedge(edges[i], true));
        }
        VectorAnimationComplex::Cycle cycle(halfEdges);
        VectorAnimationComplex::KeyFace * face = global()->currentVAC()->newKeyFace(cycle);
        face->setColor(pa.fill);
    }

    return true;
}

bool SvgParser::readCircle_(XmlStreamReader &xml)
{
    // Check to make sure we are reading a circle object
    if(xml.name() != "circle") return true;

    bool okay;

    // Get attributes

    // Center X position
    double cx = xml.attributes().hasAttribute("cx") ? xml.attributes().value("cx").toDouble(&okay) : 0;
    if(!okay) cx = 0;

    // Center Y position
    double cy = xml.attributes().hasAttribute("cy") ? xml.attributes().value("cy").toDouble(&okay) : 0;
    if(!okay) cy = 0;

    // Radius
    double r = xml.attributes().value("r").toDouble(&okay);
    // Error, radius isn't a real number
    if(!okay) return false;

    // Negative radius results in an error
    if(r < 0) return false;
    // A radius of 0 does not result in an error, but disables rendering of the object
    if(r == 0) return true;

    // Get presentation attributes
    SvgPresentationAttributes pa(xml, *this);

    // Build verticies and edges
    QVector<VectorAnimationComplex::KeyVertex *> v;
    v.push_back(global()->currentVAC()->newKeyVertex(global()->activeTime(), Eigen::Vector2d(cx + r, cy)));
    v.push_back(global()->currentVAC()->newKeyVertex(global()->activeTime(), Eigen::Vector2d(cx, cy + r)));
    v.push_back(global()->currentVAC()->newKeyVertex(global()->activeTime(), Eigen::Vector2d(cx - r, cy)));
    v.push_back(global()->currentVAC()->newKeyVertex(global()->activeTime(), Eigen::Vector2d(cx, cy - r)));
    QVector<SculptCurve::Curve<VectorAnimationComplex::EdgeSample> > c(4);
    QVector<VectorAnimationComplex::KeyEdge *> e(4);

    // The number of sampling segments in a quarter of a circle
    double quarterSegments = qCeil(r * M_PI_2 / c[0].ds());
    // Angle with at most arc length of ds
    double dsAngle = M_PI_2 / quarterSegments;

    for(int i = 0; i < 4; i++) {
        // Set vertex color
        v[i]->setColor(pa.stroke);

        // Create curve
        c[i].setEndPoints(VectorAnimationComplex::EdgeSample(v[i]->pos()[0], v[i]->pos()[1], pa.strokeWidth), VectorAnimationComplex::EdgeSample(v[(i+1)%4]->pos()[0], v[(i+1)%4]->pos()[1], pa.strokeWidth));
        c[i].beginSketch(VectorAnimationComplex::EdgeSample(v[i]->pos()[0], v[i]->pos()[1], pa.strokeWidth));
        for(int j = 1; j < quarterSegments; j++) {
            c[i].continueSketch(VectorAnimationComplex::EdgeSample(cx + r * qCos(j * dsAngle + M_PI_2 * i), cy + r * qSin(j * dsAngle + M_PI_2 * i), pa.strokeWidth));
        }
        c[i].continueSketch(VectorAnimationComplex::EdgeSample(v[(i+1)%4]->pos()[0], v[(i+1)%4]->pos()[1], pa.strokeWidth));
        c[i].endSketch();

        // Create KeyEdge
        e[i] = global()->currentVAC()->newKeyEdge(global()->activeTime(), v[i], v[(i+1)%4], (new VectorAnimationComplex::LinearSpline(c[i])), pa.strokeWidth);
        e[i]->setColor(pa.stroke);
    }

    // Add fill
    if(xml.attributes().value("fill").trimmed() != "none")
    {
        QList<VectorAnimationComplex::KeyHalfedge> edges;
        for(VectorAnimationComplex::KeyEdge * edge : e) {
            edges.append(VectorAnimationComplex::KeyHalfedge(edge, true));
        }
        VectorAnimationComplex::Cycle cycle(edges);
        VectorAnimationComplex::KeyFace * face = global()->currentVAC()->newKeyFace(cycle);
        face->setColor(pa.fill);
    }

    return true;
}

// TODO properly implement this instead of using a stretched circle
bool SvgParser::readEllipse_(XmlStreamReader &xml)
{
    // Check to make sure we are reading an ellipse object
    if(xml.name() != "ellipse") return true;

    bool okay;

    double r = 500;

    // Get attributes

    // Center X position
    double cx = xml.attributes().hasAttribute("cx") ? xml.attributes().value("cx").toDouble(&okay) : 0;
    if(!okay) cx = 0;

    // Center Y position
    double cy = xml.attributes().hasAttribute("cy") ? xml.attributes().value("cy").toDouble(&okay) : 0;
    if(!okay) cy = 0;

    // X radius
    double rx = xml.attributes().value("rx").toDouble(&okay);
    // Error, x radius isn't a real number
    if(!okay) return false;

    // Y radius
    double ry = xml.attributes().value("ry").toDouble(&okay);
    // Error, y radius isn't a real number
    if(!okay) return false;

    // Negative x or y radius results in an error
    if(rx < 0 || ry < 0) return false;
    // A x or y radius of 0 does not result in an error, but disables rendering of the object
    if(rx == 0 || ry == 0) return true;

    // Get presentation attributes
    SvgPresentationAttributes pa(xml, *this);

    // Build verticies and edges
    QVector<VectorAnimationComplex::KeyVertex *> v;
    v.push_back(global()->currentVAC()->newKeyVertex(global()->activeTime(), Eigen::Vector2d(cx + r, cy)));
    v.push_back(global()->currentVAC()->newKeyVertex(global()->activeTime(), Eigen::Vector2d(cx, cy + r)));
    v.push_back(global()->currentVAC()->newKeyVertex(global()->activeTime(), Eigen::Vector2d(cx - r, cy)));
    v.push_back(global()->currentVAC()->newKeyVertex(global()->activeTime(), Eigen::Vector2d(cx, cy - r)));
    QVector<SculptCurve::Curve<VectorAnimationComplex::EdgeSample> > c(4);
    QVector<VectorAnimationComplex::KeyEdge *> e(4);

    // The number of sampling segments in a quarter of a circle
    double quarterSegments = qCeil(r * M_PI_2 / c[0].ds());
    // Angle with at most arc length of ds
    double dsAngle = M_PI_2 / quarterSegments;

    Eigen::Translation2d pivot(cx, cy);
    Eigen::Affine2d transformation(Eigen::Scaling(0.002 * rx, 0.002 * ry));
    transformation = pivot * transformation * pivot.inverse();

    for(int i = 0; i < 4; i++) {
        // Set vertex color
        v[i]->setColor(pa.stroke);

        // Create curve
        c[i].setEndPoints(VectorAnimationComplex::EdgeSample(v[i]->pos()[0], v[i]->pos()[1], pa.strokeWidth), VectorAnimationComplex::EdgeSample(v[(i+1)%4]->pos()[0], v[(i+1)%4]->pos()[1], pa.strokeWidth));
        c[i].beginSketch(VectorAnimationComplex::EdgeSample(v[i]->pos()[0], v[i]->pos()[1], pa.strokeWidth));
        for(int j = 1; j < quarterSegments; j++) {
            c[i].continueSketch(VectorAnimationComplex::EdgeSample(cx + r * qCos(j * dsAngle + M_PI_2 * i), cy + r * qSin(j * dsAngle + M_PI_2 * i), pa.strokeWidth));
        }
        c[i].continueSketch(VectorAnimationComplex::EdgeSample(v[(i+1)%4]->pos()[0], v[(i+1)%4]->pos()[1], pa.strokeWidth));
        c[i].endSketch();

        // Create KeyEdge
        e[i] = global()->currentVAC()->newKeyEdge(global()->activeTime(), v[i], v[(i+1)%4], (new VectorAnimationComplex::LinearSpline(c[i])), pa.strokeWidth);
        e[i]->setColor(pa.stroke);

        e[i]->prepareAffineTransform();
        e[i]->performAffineTransform(transformation);
    }

    for(VectorAnimationComplex::KeyVertex * vertex : v) {
        vertex->prepareAffineTransform();
        vertex->performAffineTransform(transformation);
    }

    for(VectorAnimationComplex::KeyVertex * vertex : v) {
        vertex->correctEdgesGeometry();
    }

    // Add fill
    if(xml.attributes().value("fill").trimmed() != "none")
    {
        QList<VectorAnimationComplex::KeyHalfedge> edges;
        for(VectorAnimationComplex::KeyEdge * edge : e) {
            edges.append(VectorAnimationComplex::KeyHalfedge(edge, true));
        }
        VectorAnimationComplex::Cycle cycle(edges);
        VectorAnimationComplex::KeyFace * face = global()->currentVAC()->newKeyFace(cycle);
        face->setColor(pa.fill);
    }

    return true;
}

bool SvgParser::readPath_(XmlStreamReader &xml)
{
    // Check to make sure we are reading a path object
    if(xml.name() != "path" || xml.attributes().hasAttribute("d")) return true;

    // Get attributes

    // Get presentation attributes
    SvgPresentationAttributes pa(xml, *this);

    QString d = xml.attributes().value("d").toString();

    for(int startPos = 0; startPos < d.length();) {
        switch(d.at(startPos).cell()) {
        case 'M':
            break;
        }
    }

    return true;
}

void SvgParser::readSvg_(XmlStreamReader & xml)
{
    while (xml.readNextStartElement())
    {
        if(xml.name() == "svg") {
            while(xml.readNextStartElement())
            {
                if(xml.name() == "rect")
                {
                    if(!readRect_(xml)) return;
                }
                else if(xml.name() == "line")
                {
                    if(!readLine_(xml)) return;
                }
                else if(xml.name() == "polyline")
                {
                    if(!readPolyline_(xml)) return;
                }
                else if(xml.name() == "polygon")
                {
                    if(!readPolygon_(xml)) return;
                }
                else if(xml.name() == "circle")
                {
                    if(!readCircle_(xml)) return;
                }
                else if(xml.name() == "ellipse")
                {
                    if(!readEllipse_(xml)) return;
                }

                xml.skipCurrentElement();
            }
        }
        else {
            qDebug() << xml.name();
        }

        xml.skipCurrentElement();
    }
}

SvgPresentationAttributes::SvgPresentationAttributes(XmlStreamReader &xml, SvgParser & parser) {
    bool okay;

    // Stroke width
    strokeWidth = xml.attributes().hasAttribute("stroke-width") ? qMax(0.0, xml.attributes().value("stroke-width").toDouble(&okay)) : 1;
    if(!okay) strokeWidth = 1;

    // Fill (color)
    fill = xml.attributes().hasAttribute("fill") ? parser.parseColor_(xml.attributes().value("fill").toString()) : QColor("black");
    if(!fill.isValid()) fill = QColor("black");

    // Stroke (color)
    stroke = xml.attributes().hasAttribute("stroke") ? parser.parseColor_(xml.attributes().value("stroke").toString()) : QColor("none");
    if(!stroke.isValid()) fill = QColor("none");

    // Fill opacity
    double fillOpacity = xml.attributes().hasAttribute("fill-opacity") ? qBound(0.0, xml.attributes().value("fill-opacity").toDouble(&okay), 1.0) : 1;
    if(!okay) fillOpacity = 1;

    // Stroke opacity
    double strokeOpacity = xml.attributes().hasAttribute("stroke-opacity") ? qBound(0.0, xml.attributes().value("stroke-opacity").toDouble(&okay), 1.0) : 1;
    if(!okay) strokeOpacity = 1;

    // Opacity (whole object)
    double opacity = xml.attributes().hasAttribute("opacity") ? qBound(0.0, xml.attributes().value("opacity").toDouble(&okay), 1.0) : 1;
    if(!okay) opacity = 1;

    // Opacities appear to stack
    fill.setAlphaF(fill.alphaF() * fillOpacity * opacity);
    stroke.setAlphaF(stroke.alphaF() * strokeOpacity * opacity);

    // If stroke is none, then just set stroke opacity to 0
    if(xml.attributes().value("stroke").trimmed() == "none")
    {
        stroke.setAlphaF(0);
    }
}

// For coordinate pair detection *with optional comma-wsp* which applies only to path elements, not polylines/polygons
Eigen::Vector2d SvgParser::getNextCoordinatePair(QString & orig, bool * ok)
{
    QString s(static_cast<const QString>(orig));

    // Find first number
    QRegExp realNumberExp("[+\\-]?(([0-9]+\\.?[0-9]*) | (\\.[0-9]+))([Ee][0-9]+)?");
    if(realNumberExp.indexIn(s) != 0)
    {
        *ok = false;
        return Eigen::Vector2d(0, 0);
    }
    double x = realNumberExp.cap(0).toDouble(ok);
    if(!ok)
    {
        return Eigen::Vector2d(0, 0);
    }
    s.remove(0, realNumberExp.cap(0).length());

    // Move past whitespace
    while(!s.isEmpty() && (s[0] == 0x20 || s[0] == 0x9 || s[0] == 0xD || s[0] == 0xA)) s.remove(0, 1);
    // Check for comma
    if(s[0] == ',')
    {
        s.remove(0, 1);
        // Move past whitespace
        while(!s.isEmpty() && (s[0] == 0x20 || s[0] == 0x9 || s[0] == 0xD || s[0] == 0xA)) s.remove(0, 1);
    }

    // Find second number
    if(realNumberExp.indexIn(s) != 0)
    {
        *ok = false;
        return Eigen::Vector2d(0, 0);
    }
    double y = realNumberExp.cap(0).toDouble(ok);
    if(!ok)
    {
        return Eigen::Vector2d(0, 0);
    }
    s.remove(0, realNumberExp.cap(0).length());

    orig = s;
    *ok = true;
    return Eigen::Vector2d(x, y);
}

/*void parsePathData() {

}*/

template<class T>
bool PotentialPoint<T>::isSmooth()
{
    // Endpoints are not smooth
    /*if(left_ < 0 || right_ < 0) return false;

    if(qAbs(left_ - right_) < angleThreshold) return true;*/
    return false;
}
