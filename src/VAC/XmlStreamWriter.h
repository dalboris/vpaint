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

#ifndef XMLSTREAMWRITER_H
#define XMLSTREAMWRITER_H

#include <QXmlStreamWriter>
#include "VAC/vpaint_global.h"

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

class Q_VPAINT_EXPORT XmlStreamWriter : public QXmlStreamWriter
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
