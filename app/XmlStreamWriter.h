// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef XMLSTREAMWRITER_H
#define XMLSTREAMWRITER_H

#include <QXmlStreamWriter>

class XmlStreamWriter : public QXmlStreamWriter
{
public:
    XmlStreamWriter(QIODevice * device);
    ~XmlStreamWriter();

    // Writes whatever you want directly to the device.
    // This function ignores XML syntax, be careful when using it.
    void write(const QString & string) const;

    // Writes a start element
    void writeStartElement(const QString & qualifiedName);
    void writeEndElement();

    // Writes an element attributes
    void writeAttribute(const QString & qualifiedName, const QString & value);

private:
    int indentLevel_;
};


#endif // XMLSTREAMWRITER_H
