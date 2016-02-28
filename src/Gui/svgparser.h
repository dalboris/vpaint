#ifndef SVGPARSER_H
#define SVGPARSER_H

#include <QtGlobal>
#include <XmlStreamReader.h>
#include <QColor>

class QString;

class SVGParser
{
public:
    SVGParser();
//private:
    QColor parseColor_(QString s);
    bool readRect_(XmlStreamReader & xml);
    bool readLine_(XmlStreamReader & xml);
    void readSVG_(XmlStreamReader & xml);
};

class SVGPresentationAttributes
{
public:
    SVGPresentationAttributes(XmlStreamReader & xml, SVGParser & parser);
    QColor fill, stroke;
    qreal strokeWidth;
};

#endif // SVGPARSER_H
