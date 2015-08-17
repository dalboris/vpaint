// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "EdgeCell.h"
#include "VertexCell.h"
#include "FaceCell.h"
#include "VAC.h"
#include "../Random.h"
#include "../DevSettings.h"
#include "../Global.h"
#include <cmath>
#include <QtDebug>
#include <QTextStream>
#include "../SaveAndLoad.h"
#include "../CssColor.h"
#include "EdgeGeometry.h"

namespace VectorAnimationComplex
{

EdgeCell::EdgeCell(VAC * vac) :
    Cell(vac)
{
    QColor edgeColor = global()->edgeColor();
    color_[0] = edgeColor.redF();
    color_[1] = edgeColor.greenF();
    color_[2] = edgeColor.blueF();
    color_[3] = edgeColor.alphaF();
    CssColor cssColor(edgeColor.red(),edgeColor.green(),edgeColor.blue(),edgeColor.alphaF());
    style_.set("color",cssColor.toString());

    // highlighted/selected color
    colorSelected_[0] = 1;
    colorSelected_[1] = 0;
    colorSelected_[2] = 0;
    colorSelected_[3] = 1;
}

EdgeCell::EdgeCell(VAC * vac, QTextStream & in) :
    Cell(vac, in)
{
    // highlighted/selected color
    colorSelected_[0] = 1;
    colorSelected_[1] = 0;
    colorSelected_[2] = 0;
    colorSelected_[3] = 1;
}

EdgeCell::EdgeCell(VAC * vac, XmlStreamReader & xml) :
    Cell(vac, xml)
{
    // highlighted/selected color
    colorSelected_[0] = 1;
    colorSelected_[1] = 0;
    colorSelected_[2] = 0;
    colorSelected_[3] = 1;
}

bool EdgeCell::isPickableCustom(Time /*time*/) const
{
    const bool areEdgesPickable = true;
    if(areEdgesPickable && global()->toolMode() == Global::SELECT)
        return true;
    else if(global()->toolMode() == Global::PAINT)
        return true;
    else
        return false;
}

void EdgeCell::read2ndPass()
{
}

void EdgeCell::save_(QTextStream & /*out*/)
{
}

void EdgeCell::write_(XmlStreamWriter & /*xml*/) const
{

}

CellSet EdgeCell::spatialBoundary() const
{
    if(isClosed())
    {
        return CellSet();
    }
    else
    {
        CellSet left = startVertices();
        CellSet right = endVertices();
        left.unite(right);
        return left;
    }
}

EdgeCellSet EdgeCell::incidentEdges() const
{
    CellSet incidentCells;
    foreach(Cell * c, spatialBoundary())
        foreach(Cell * d, c->spatialStar())
            if(d != this)
            incidentCells << d;
    return incidentCells;
}

void EdgeCell::triangulate(double /*width*/, Time /*time*/, Triangles & out)
{
    out.clear();
}

void EdgeCell::triangulate(Time /*time*/, Triangles & out)
{
    out.clear();
}

void EdgeCell::clearCachedGeometry_()
{
    triangles_.clear();
    trianglesTopo_.clear();
}

Triangles & EdgeCell::triangles(Time time)
{
    int nSixtiethOfFrame = std::floor(time.floatTime() * 60 + 0.5);
    if(!triangles_.contains(nSixtiethOfFrame))
    {
        triangles_[nSixtiethOfFrame] = Triangles();
        triangulate(time, triangles_[nSixtiethOfFrame]);
    }

    return triangles_[nSixtiethOfFrame];
}

Triangles & EdgeCell::triangles(double width, Time time)
{
    int nSixtiethOfFrame = std::floor(time.floatTime() * 60 + 0.5);
    QPair<int,double> pair = qMakePair(nSixtiethOfFrame, width);

    if(!trianglesTopo_.contains(pair))
    {
        trianglesTopo_[pair] = Triangles();
        triangulate(width, time, trianglesTopo_[pair]);
    }

    return trianglesTopo_[pair];
}

void EdgeCell::drawRaw(Time time, ViewSettings & /*viewSettings*/)
{
    triangles(time).draw();
}

void EdgeCell::drawRawTopology(Time time, ViewSettings & viewSettings)
{
    bool screenRelative = viewSettings.screenRelative();
    if(screenRelative)
    {
        triangles(viewSettings.edgeTopologyWidth() / viewSettings.zoom(), time).draw();
    }
    else
    {
        triangles(viewSettings.edgeTopologyWidth(), time).draw();
    }
}

EdgeSample EdgeCell::startSample(Time time) const
{
    QList<EdgeSample> sampling = getSampling(time);
    if(sampling.isEmpty())
        return EdgeSample();
    else
        return sampling.first();
}

EdgeSample EdgeCell::endSample(Time time) const
{
    QList<EdgeSample> sampling = getSampling(time);
    if(sampling.isEmpty())
        return EdgeSample();
    else
        return sampling.last();
}

bool EdgeCell::intersectsRectangle(Time t, double x0, double x1, double y0, double y1)
{
    return triangles(t).intersectsRectangle(x0, x1, y0, y1);
}

void EdgeCell::exportSVG(Time t, QTextStream & out)
{
    QList<EdgeSample> samples = getSampling(t);
    LinearSpline ls(samples);
    if(isClosed())
        ls.makeLoop();

    out << "<path d=\"";
    ls.exportSVG(out);
    out << "\" style=\""
        << "fill:rgb("
        << (int) (color_[0]*255) << ","
        << (int) (color_[1]*255) << ","
        << (int) (color_[2]*255) << ");"
        << "fill-opacity:" << color_[3] << ";"
        << "fill-rule:nonzero;stroke:none\" />\n";
}

EdgeCell::~EdgeCell()
{
}

EdgeCell::EdgeCell(EdgeCell * other) :
    Cell(other)
{
    // highlighted/selected color
    colorSelected_[0] = 1;
    colorSelected_[1] = 0;
    colorSelected_[2] = 0;
    colorSelected_[3] = 1;
}

void EdgeCell::remapPointers(VAC * /*newVAC*/)
{
}

bool EdgeCell::checkEdge_() const
{
    // todo
    return true;
}

}
