// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "TransformTool.h"

#include "BoundingBox.h"
#include "OpenGL.h"
#include "Picking.h"
#include "Cell.h"

#include <QtDebug>

namespace VectorAnimationComplex
{

TransformTool::TransformTool() :
    idOffset_(0),
    hovered_(None)
{
}

void TransformTool::setIdOffset(int idOffset)
{
    idOffset_ = idOffset;
}

TransformTool::WidgetId TransformTool::hovered() const
{
    return hovered_;
}

void TransformTool::drawScaleWidget_(double x, double y, double size, WidgetId id, ViewSettings & viewSettings) const
{
    size = size / viewSettings.zoom();

    const double fillColor[] = {0.8, 0.8, 0.8, 0.2};
    const double strokeColor[] = {0.5, 0.5, 0.5, 0.2};

    const double fillColorHighlighted[] = {1.0, 0.8, 0.8, 0.8};
    const double strokeColorHighlighted[] = {1.0, 0.5, 0.5, 0.8};

    // Fill
    if(hovered_ == id)
    {
        glColor4dv(fillColorHighlighted);
    }
    else
    {
        glColor4dv(fillColor);
    }
    glBegin(GL_QUADS);
    {
        glVertex2d(x - size, y - size);
        glVertex2d(x + size, y - size);
        glVertex2d(x + size, y + size);
        glVertex2d(x - size, y + size);
    }
    glEnd();

    // Stroke
    if(hovered_ == id)
    {
        glColor4dv(strokeColorHighlighted);
    }
    else
    {
        glColor4dv(strokeColor);
    }
    glBegin(GL_LINE_LOOP);
    {
        glVertex2d(x - size, y - size);
        glVertex2d(x + size, y - size);
        glVertex2d(x + size, y + size);
        glVertex2d(x - size, y + size);
    }
    glEnd();
}

void TransformTool::drawPickScaleWidget_(double x, double y, double size, WidgetId id, ViewSettings &viewSettings) const
{
    size = size / viewSettings.zoom();

    glPickColor_(id);
    glBegin(GL_QUADS);
    {
        glVertex2d(x - size, y - size);
        glVertex2d(x + size, y - size);
        glVertex2d(x + size, y + size);
        glVertex2d(x - size, y + size);
    }
    glEnd();
}

void TransformTool::glPickColor_(WidgetId id) const
{
    Picking::glColor(idOffset_ + id - MIN_WIDGET_ID);
}

namespace
{
const double scaleWidgetCornerSize = 10;
const double scaleWidgetEdgeSize = 5;
const double lineWidth = 1.0;

const double boundingBoxColor[] = {0.5, 0.5, 0.5, 0.5};
}

void TransformTool::draw(const CellSet & cells, Time time, ViewSettings & viewSettings) const
{
    // Compute selection bounding box at current time
    BoundingBox bb;
    for (CellSet::ConstIterator it = cells.begin(); it != cells.end(); ++it)
    {
        bb.unite((*it)->boundingBox(time));
    }

    // Draw bounding box and transform widgets
    if (bb.isProper())
    {
        glLineWidth(lineWidth);

        // Bounding box
        glColor4dv(boundingBoxColor);
        glBegin(GL_LINE_LOOP);
        {
            glVertex2d(bb.xMin(), bb.yMin());
            glVertex2d(bb.xMax(), bb.yMin());
            glVertex2d(bb.xMax(), bb.yMax());
            glVertex2d(bb.xMin(), bb.yMax());
        }
        glEnd();

        // Scale widgets
        drawScaleWidget_(bb.xMin(), bb.yMin(), scaleWidgetCornerSize, TopLeftScale, viewSettings);
        drawScaleWidget_(bb.xMax(), bb.yMin(), scaleWidgetCornerSize, TopRightScale, viewSettings);
        drawScaleWidget_(bb.xMax(), bb.yMax(), scaleWidgetCornerSize, BottomRightScale, viewSettings);
        drawScaleWidget_(bb.xMin(), bb.yMax(), scaleWidgetCornerSize, BottomLeftScale, viewSettings);
        drawScaleWidget_(0.5*(bb.xMin()+bb.xMax()), bb.yMin(), scaleWidgetEdgeSize, TopScale, viewSettings);
        drawScaleWidget_(bb.xMax(), 0.5*(bb.yMin()+bb.yMax()), scaleWidgetEdgeSize, RightScale, viewSettings);
        drawScaleWidget_(0.5*(bb.xMin()+bb.xMax()), bb.yMax(), scaleWidgetEdgeSize, BottomScale, viewSettings);
        drawScaleWidget_(bb.xMin(), 0.5*(bb.yMin()+bb.yMax()), scaleWidgetEdgeSize, LeftScale, viewSettings);
    }
}


void TransformTool::drawPick(const CellSet & cells, Time time, ViewSettings & viewSettings) const
{
    // Compute selection bounding box at current time
    BoundingBox bb;
    for (CellSet::ConstIterator it = cells.begin(); it != cells.end(); ++it)
    {
        bb.unite((*it)->boundingBox(time));
    }

    // Draw bounding box
    if (bb.isProper())
    {
        // Scale widgets
        drawPickScaleWidget_(bb.xMin(), bb.yMin(), scaleWidgetCornerSize, TopLeftScale, viewSettings);
        drawPickScaleWidget_(bb.xMax(), bb.yMin(), scaleWidgetCornerSize, TopRightScale, viewSettings);
        drawPickScaleWidget_(bb.xMax(), bb.yMax(), scaleWidgetCornerSize, BottomRightScale, viewSettings);
        drawPickScaleWidget_(bb.xMin(), bb.yMax(), scaleWidgetCornerSize, BottomLeftScale, viewSettings);
        drawPickScaleWidget_(0.5*(bb.xMin()+bb.xMax()), bb.yMin(), scaleWidgetEdgeSize, TopScale, viewSettings);
        drawPickScaleWidget_(bb.xMax(), 0.5*(bb.yMin()+bb.yMax()), scaleWidgetEdgeSize, RightScale, viewSettings);
        drawPickScaleWidget_(0.5*(bb.xMin()+bb.xMax()), bb.yMax(), scaleWidgetEdgeSize, BottomScale, viewSettings);
        drawPickScaleWidget_(bb.xMin(), 0.5*(bb.yMin()+bb.yMax()), scaleWidgetEdgeSize, LeftScale, viewSettings);
    }
}

void TransformTool::setHoveredObject(int id)
{
    int widgetId = id - idOffset_ + MIN_WIDGET_ID;

    if (widgetId >= MIN_WIDGET_ID &&
        widgetId <= MAX_WIDGET_ID)
    {
        hovered_ = static_cast<WidgetId>(widgetId);
    }
    else
    {
        setNoHoveredObject();
    }
}

void TransformTool::setNoHoveredObject()
{
    hovered_ = None;
}

void TransformTool::beginTransform(const CellSet & cells, double x0, double y0, Time time)
{
    qDebug() << "beginTransform";
}

void TransformTool::continueTransform(const CellSet & cells, double x, double y)
{
    qDebug() << "continueTransform";
}

void TransformTool::endTransform(const CellSet & cells)
{
    qDebug() << "endTransform";
}

}
