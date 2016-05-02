// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef XMLSTREAMWRITER_H
#define XMLSTREAMWRITER_H

#include <QXmlStreamWriter>

/// \class XmlStreamWriter
/// Writes an XML document to a file.
///
/// This class overrides Qt default behaviour to achieve desired indenting style,
/// for better human-readability in our use-case.
///
/// For instance, instead of:
///
///     <tag attr1="value1" attr2="value2"/>
///
/// We want:
///
///     <tag
///       attr1="value1"
///       attr2="value2"/>
///
/// Also, we want to allow writing multiline attribute values without escaping '\n',
/// such that instead of:
///
///     <tag
///       attr="a long attribute&#10;spanning three&#10;different lines"/>
///
/// We get:
///
///     <tag
///       attr="a long attribute
///             spanning three
///             different lines"/>
///
/// This is XML compliant but one should be aware that newlines characters will be
/// replaced by whitespaces by any compliant XML parser, i.e. it will be interpreted as:
///
///     "a long attribute             spanning three             different lines"
///
/// This is OK because in the specification of the VEC file formats, newlines in attributes
/// are never significant, and consecutive whitespaces are equivalent to single whitespaces.

class XmlStreamWriter : public QXmlStreamWriter
{
public:
    XmlStreamWriter(QIODevice * device);
    ~XmlStreamWriter();

    // Writes a start element
    void writeStartElement(const QString & qualifiedName);
    void writeEndElement();

    // Writes an element attributes
    void writeAttribute(const QString & qualifiedName, const QString & value);
    void writeAttribute(const QXmlStreamAttribute & attribute);
    void writeAttributes(const QXmlStreamAttributes & attributes);

private:
    int indentLevel_;

    // Raw-write to device, without escaping XML characters
    void write(const QString & string) const;

    // Escape special characters
    static QString escaped(const QString & s);

    // Escape special characters except newlines
    static QString escapedExceptNewlines(const QString & s);

};


#endif // XMLSTREAMWRITER_H
