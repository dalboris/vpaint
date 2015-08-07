// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

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

void XmlStreamWriter::writeAttribute(const QString & qualifiedName, const QString & value)
{
    const QChar space(' ');
    const int numSpaces = indentLevel_*autoFormattingIndent() - 1;

    QString indent("\n");
    for(int i=0; i<numSpaces; ++i)
        indent += space;

    write(indent);
    QXmlStreamWriter::writeAttribute(qualifiedName, value);
}
