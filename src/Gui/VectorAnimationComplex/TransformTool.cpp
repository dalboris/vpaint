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

void TransformTool::draw(const CellSet & cells, Time time, ViewSettings & viewSettings) const
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
        if (hovered())
            glColor4d(1.0,0.0,0.0,1.0);
        else
            glColor4d(0.5, 0.5, 0.5, 0.5);

        glLineWidth(1);
        glBegin(GL_LINE_LOOP);
        {
            glVertex2d(bb.xMin(), bb.yMin());
            glVertex2d(bb.xMax(), bb.yMin());
            glVertex2d(bb.xMax(), bb.yMax());
            glVertex2d(bb.xMin(), bb.yMax());
        }
        glEnd();
    }
}

void TransformTool::glPickColor_(WidgetId id) const
{
    Picking::glColor(idOffset_ + id - MIN_WIDGET_ID);
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
        glPickColor_(TopLeftScale);
        glLineWidth(3);
        glBegin(GL_LINE_LOOP);
        {
            glVertex2d(bb.xMin(), bb.yMin());
            glVertex2d(bb.xMax(), bb.yMin());
            glVertex2d(bb.xMax(), bb.yMax());
            glVertex2d(bb.xMin(), bb.yMax());
        }
        glEnd();
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
