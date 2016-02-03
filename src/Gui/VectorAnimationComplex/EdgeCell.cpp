// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

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

void EdgeCell::clearCachedGeometry_()
{
    Cell::clearCachedGeometry_();
    trianglesTopo_.clear();
}

void EdgeCell::computeOutlineBoundingBox_(Time t, BoundingBox & out) const
{
    const QList<EdgeSample> samples = getSampling(t);

    out = BoundingBox();
    for (int i = 0; i<samples.size(); ++i)
        out.unite(BoundingBox(samples[i].x(), samples[i].y()));
}

const Triangles & EdgeCell::triangles(double width, Time time) const
{
    // Get cache key
    QPair<int,double> key = qMakePair(std::floor(time.floatTime() * 60 + 0.5), width);

    // Compute triangles if not yet cached
    if(!trianglesTopo_.contains(key))
        triangulate_(width, time, trianglesTopo_[key]);

    // Return cached triangles
    return trianglesTopo_[key];
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
