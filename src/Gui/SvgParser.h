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
