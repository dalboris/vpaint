// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef XMLSTREAMREADER_H
#define XMLSTREAMREADER_H

#include <QXmlStreamReader>

class XmlStreamReader: public QXmlStreamReader
{
public:
    XmlStreamReader(QIODevice * device);
    ~XmlStreamReader();
};

#endif // XMLSTREAMREADER_H
