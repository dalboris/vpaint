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

#include "XmlStreamConverter_1_0_to_1_6.h"

#include "../../XmlStreamReader.h"
#include "../../XmlStreamWriter.h"

#include <QString>
#include <QRegularExpression>

namespace
{

void changeAttributeValue(QXmlStreamAttributes & attributes,
                          const QString & qualifiedName,
                          const QString & newValue)
{
    for (int i=0; i<attributes.size(); ++i)
    {
        if (attributes[i].qualifiedName() == qualifiedName)
        {
            attributes[i] = QXmlStreamAttribute(qualifiedName, newValue);
        }
    }
}

void removeAttribute(QXmlStreamAttributes & attributes,
                     const QString & qualifiedName)
{
    QXmlStreamAttributes newAttributes;
    for (int i=0; i<attributes.size(); ++i)
    {
        if (attributes[i].qualifiedName() != qualifiedName)
        {
            newAttributes << attributes[i];
        }
    }
    attributes = newAttributes;
}

void insertAttribute(QXmlStreamAttributes & attributes,
                     int i,
                     const QString & qualifiedName,
                     const QString & value)
{
    attributes.insert(i, QXmlStreamAttribute(qualifiedName, value));
}

void prependAttribute(QXmlStreamAttributes & attributes,
                     const QString & qualifiedName,
                     const QString & value)
{
    attributes.prepend(QXmlStreamAttribute(qualifiedName, value));
}

void appendAttribute(QXmlStreamAttributes & attributes,
                     const QString & qualifiedName,
                     const QString & value)
{
    attributes.append(QXmlStreamAttribute(qualifiedName, value));
}

void convertColorStyleToAttribute(QXmlStreamAttributes & attributes)
{
    for (int i=0; i<attributes.size(); ++i)
    {
        if (attributes[i].qualifiedName() == "style")
        {
            QString colorValue;

            // Extract color
            QString styleValue = attributes[i].value().toString();
            QRegularExpression re("(;|^)color:([^;]*)(;|$)");
            QRegularExpressionMatch match = re.match(styleValue);
            if (match.hasMatch()) {
                colorValue = match.captured(2);
            }

            attributes[i] = QXmlStreamAttribute("color", colorValue);
        }
    }
}

}


XmlStreamConverter_1_0_to_1_6::XmlStreamConverter_1_0_to_1_6(XmlStreamReader & in, XmlStreamWriter & out) :
    XmlStreamConverter(in, out)
{
}

void XmlStreamConverter_1_0_to_1_6::begin()
{
    // Start XML Document
    out().writeStartDocument();

    // Header
    out().writeComment(" Created with VPaint (http://www.vpaint.org) ");
    out().writeCharacters("\n\n");
}

void XmlStreamConverter_1_0_to_1_6::end()
{
    // End XML Document
    out().writeEndDocument();
}

void XmlStreamConverter_1_0_to_1_6::pre()
{
    QString name = in().name().toString();
    QXmlStreamAttributes attrs = in().attributes();

    if (name == "vec")
    {
        QXmlStreamAttributes attrs = in().attributes();
        changeAttributeValue(attrs, "version", "1.6");

        out().writeStartElement(name);
        out().writeAttributes(attrs);
    }
    else if (name == "playback")
    {
        QString framerangeValue;
        if (attrs.hasAttribute("firstframe"))
            framerangeValue += attrs.value("firstframe").toString();
        else
            framerangeValue += "0";
        framerangeValue += " ";
        if (attrs.hasAttribute("lastframe"))
            framerangeValue += attrs.value("lastframe").toString();
        else
            framerangeValue += "0";

        removeAttribute(attrs, "firstframe");
        removeAttribute(attrs, "lastframe");
        prependAttribute(attrs, "framerange", framerangeValue);

        out().writeStartElement(name);
        out().writeAttributes(attrs);
    }
    else if (name == "canvas")
    {
        QString positionValue;
        if (attrs.hasAttribute("left"))
            positionValue += attrs.value("left").toString();
        else
            positionValue += "0";
        positionValue += " ";
        if (attrs.hasAttribute("top"))
            positionValue += attrs.value("top").toString();
        else
            positionValue += "0";

        QString sizeValue;
        if (attrs.hasAttribute("width"))
            sizeValue += attrs.value("width").toString();
        else
            sizeValue += "0";
        sizeValue += " ";
        if (attrs.hasAttribute("height"))
            sizeValue += attrs.value("height").toString();
        else
            sizeValue += "0";

        removeAttribute(attrs, "left");
        removeAttribute(attrs, "top");
        removeAttribute(attrs, "width");
        removeAttribute(attrs, "height");
        appendAttribute(attrs, "position", positionValue);
        appendAttribute(attrs, "size", sizeValue);

        out().writeStartElement(name);
        out().writeAttributes(attrs);
    }
    else if (name == "layer")
    {
        QString backgroundColorValue;

        if (attrs.hasAttribute("style"))
        {
            // Extract background color
            QString styleValue = attrs.value("style").toString();
            QRegularExpression re("(;|^)background-color:([^;]*)(;|$)");
            QRegularExpressionMatch match = re.match(styleValue);
            if (match.hasMatch()) {
                backgroundColorValue = match.captured(2);
            }

            // Remove style attribute
            removeAttribute(attrs, "style");
        }

        out().writeStartElement(name);
        out().writeAttributes(attrs);

        // Add background as child element
        if (backgroundColorValue.length() > 0)
        {
            out().writeStartElement("background");
            out().writeAttribute("color", backgroundColorValue);
            out().writeEndElement();
        }

        out().writeStartElement("objects");
    }
    else if (name == "vertex")
    {
        QString oldPositionValue = attrs.value("position").toString();
        QStringList list = oldPositionValue.split(",");
        QString newPositionValue = list[0] + " " + list[1];

        changeAttributeValue(attrs, "position", newPositionValue);
        convertColorStyleToAttribute(attrs);

        out().writeStartElement(name);
        out().writeAttributes(attrs);
    }
    else if (name == "edge")
    {
        QString newCurveValue;

        QString oldCurveValue =  attrs.value("curve").toString();
        QRegularExpression re("xyw-dense: (.*)");
        QRegularExpressionMatch match = re.match(oldCurveValue);
        if (match.hasMatch()) {
            newCurveValue = "xywdense(" + match.captured(1) + ")";
        }

        changeAttributeValue(attrs, "curve", newCurveValue);
        convertColorStyleToAttribute(attrs);

        out().writeStartElement(name);
        out().writeAttributes(attrs);
    }
    else if (name == "face" ||
             name == "inbetweenvertex" ||
             name == "inbetweenedge" ||
             name == "inbetweenface")
    {
        convertColorStyleToAttribute(attrs);

        out().writeStartElement(name);
        out().writeAttributes(attrs);
    }
    else
    {
        out().writeStartElement(name);
        out().writeAttributes(attrs);
    }
}

void XmlStreamConverter_1_0_to_1_6::post()
{
    QString name = in().name().toString();

    if (name == "layer")
    {
        out().writeEndElement(); // end objects
        out().writeEndElement(); // end layer
    }
    else
    {
        out().writeEndElement();
    }
}
