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

#include "XmlStreamWriter.h"

XmlStreamWriter::XmlStreamWriter(QIODevice * device) :
    QXmlStreamWriter(device),
    indentLevel_(0)
{
    setAutoFormatting(true);
    setAutoFormattingIndent(2);
}

XmlStreamWriter::~XmlStreamWriter()
{

}

void XmlStreamWriter::write(const QString & string) const
{
    device()->write(string.toUtf8());
}

void XmlStreamWriter::writeStartElement(const QString & qualifiedName)
{
    ++indentLevel_;
    QXmlStreamWriter::writeStartElement(qualifiedName);
}

void XmlStreamWriter::writeEndElement()
{
    --indentLevel_;
    QXmlStreamWriter::writeEndElement();
}

void XmlStreamWriter::writeAttributes(const QXmlStreamAttributes & attributes)
{
    for (auto attribute : attributes)
    {
        writeAttribute(attribute);
    }
}

void XmlStreamWriter::writeAttribute(const QXmlStreamAttribute & attribute)
{
    writeAttribute(attribute.qualifiedName().toString(), attribute.value().toString());
}

void XmlStreamWriter::writeAttribute(const QString & qualifiedName, const QString & value)
{
    // Here is the style we want:
    //
    // Input:
    //
    // xml.writeStartElement("tag");
    // xml.writeAttribute("attr1", "value1");
    // xml.writeAttribute("attr2", "multiline\nvalue");
    // xml.writeAttribute("attr3", "value3");
    // xml.writeEndElement("tag");
    //
    // Output:
    //
    //   <tag
    //     attr1="value1"
    //     attr2="multiline
    //            value"
    //     attr3="value3"/>
    //

    // Compute indent
    const QChar space(' ');
    const int numSpaces = indentLevel_*autoFormattingIndent();
    QString indent("\n");
    for(int i=0; i<numSpaces; ++i)
        indent += space;

    // Write attribute name
    write(indent);
    write(qualifiedName);

    // Compute indent for attribute value newlines
    for(int i=0; i<qualifiedName.length()+2; ++i)
        indent += space;

    // Replace newlines in attribute value by indented new lines
    QString cleanedValue = value;
    cleanedValue.replace('\n', indent);

    // Escape special characters except newlines
    cleanedValue = escapedExceptNewlines(cleanedValue);

    // Write attribute value
    write("=\"");
    write(cleanedValue);
    write("\"");
}

// Escape special characters
QString XmlStreamWriter::escaped(const QString & s)
{
    QString res;
    res.reserve(s.size());
    for ( QChar c: s)
    {
       if (c.unicode() == '<' )
           res.append(QLatin1String("&lt;"));
       else if (c.unicode() == '>' )
           res.append(QLatin1String("&gt;"));
       else if (c.unicode() == '&' )
           res.append(QLatin1String("&amp;"));
       else if (c.unicode() == '\"' )
           res.append(QLatin1String("&quot;"));
       else if (c.unicode() == '\n')
           res.append(QLatin1String("&#10;"));
       else if (c.unicode() == '\r')
           res.append(QLatin1String("&#13;"));
       else if (c.unicode() == '\t')
           res.append(QLatin1String("&#9;"));
       else if (c.isSpace())
           res += QLatin1Char(' ');
       else
           res += QChar(c);
    }
    return res;
}

// Escape special characters except newlines
QString XmlStreamWriter::escapedExceptNewlines(const QString & s)
{
    QString res;
    res.reserve(s.size());
    for ( QChar c: s)
    {
       if (c.unicode() == '<' )
           res.append(QLatin1String("&lt;"));
       else if (c.unicode() == '>' )
           res.append(QLatin1String("&gt;"));
       else if (c.unicode() == '&' )
           res.append(QLatin1String("&amp;"));
       else if (c.unicode() == '\"' )
           res.append(QLatin1String("&quot;"));
       else if (c.unicode() == '\r')
           res.append(QLatin1String("&#13;"));
       else if (c.unicode() == '\t')
           res.append(QLatin1String("&#9;"));
       else
           res += QChar(c);
    }
    return res;
}
