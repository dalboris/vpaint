#include "SVGParser.h"
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

SVGParser::SVGParser()
{

}

// Parses color from string, will probably be moved to a class like CSSColor
// This implements the most of the W3 specifications
// found at https://www.w3.org/TR/SVG11/types.html#DataTypeColor
// It also extends the specifications in a few minor ways
// This includes more flexible whitespace and some CSS3 features (hsl)
QColor SVGParser::parseColor_(QString s)
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
bool SVGParser::readRect_(XmlStreamReader & xml)
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
    SVGPresentationAttributes pa(xml, *this);

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

bool SVGParser::readLine_(XmlStreamReader & xml) {
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
    SVGPresentationAttributes pa(xml, *this);

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

bool SVGParser::readPolyline_(XmlStreamReader &xml) {
    // Check to make sure we are reading a line object
    if(xml.name() != "polyline" || !xml.attributes().hasAttribute("points")) return true;

    bool okay;

    // Get presentation attributes
    SVGPresentationAttributes pa(xml, *this);

    // Read and split points
    // Technically the parsing of separators is a bit more complicated,
    // but this will suffice as it correctly handles all standard-conforming svgs
    QStringList points = xml.attributes().value("points").toString().split(QRegExp("[\\s,]+"), QString::SkipEmptyParts);

    // Fail if there are less than two points
    if(points.size() < 4) return false;

    QVector<VectorAnimationComplex::KeyVertex *> verticies(points.size() / 2);

    // Parse points
    for(int i = 0; i < verticies.size(); i++) {
        //qDebug() << points[i * 2] << ", " << points[i * 2 + 1];
        // X
        double x = points[i * 2].toDouble(&okay);
        if(!okay) return false;

        // Y
        double y = points[i * 2 + 1].toDouble(&okay);
        if(!okay) return false;

        //qDebug() << x << ", " << y;

        verticies[i] = global()->currentVAC()->newKeyVertex(global()->activeTime(), Eigen::Vector2d(x, y));
        verticies[i]->setColor(pa.stroke);
    }

    for(int i = 1; i < verticies.size(); i++) {
        VectorAnimationComplex::KeyEdge * e = global()->currentVAC()->newKeyEdge(global()->activeTime(), verticies[i-1], verticies[i], (new VectorAnimationComplex::LinearSpline(SculptCurve::Curve<VectorAnimationComplex::EdgeSample>(VectorAnimationComplex::EdgeSample(verticies[i-1]->pos()[0], verticies[i-1]->pos()[1], pa.strokeWidth), VectorAnimationComplex::EdgeSample(verticies[i]->pos()[0], verticies[i]->pos()[1], pa.strokeWidth)))), pa.strokeWidth);
        e->setColor(pa.stroke);
    }

    return true;
}

void SVGParser::readSVG_(XmlStreamReader & xml)
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

                xml.skipCurrentElement();
            }
        }
        else {
            qDebug() << xml.name();
        }

        xml.skipCurrentElement();
    }
}

SVGPresentationAttributes::SVGPresentationAttributes(XmlStreamReader &xml, SVGParser & parser) {
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
