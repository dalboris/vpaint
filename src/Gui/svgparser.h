#ifndef SVGPARSER_H
#define SVGPARSER_H

#include <QtGlobal>
#include <XmlStreamReader.h>

class QColor;
class QString;

class SVGParser
{
public:
    SVGParser();
//private:
    QColor parseColor_(QString s);
    bool readRect_(XmlStreamReader & xml);
    void readSVG_(XmlStreamReader & xml);
};

#endif // SVGPARSER_H
