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
#include "KeyVertex.h"
#include "KeyEdge.h"
#include "EdgeGeometry.h"
#include "VAC.h"
#include "Algorithms.h"

#include "Eigen.h"
#include <vector>
#include <cmath>

typedef Eigen::Vector2d Vec2;
typedef std::vector<Vec2, Eigen::aligned_allocator<Vec2>> Vec2Vector;

namespace VectorAnimationComplex
{

namespace
{

// Math constants
const double PI = 3.14159;
const double SQRT2 = 1.4142;

// Widget colors
const double boundingBoxColor[] = {0.5, 0.5, 0.5, 0.5};
const double fillColor[] = {0.8, 0.8, 0.8, 0.2};
const double strokeColor[] = {0.5, 0.5, 0.5, 0.2};
const double fillColorHighlighted[] = {1.0, 0.8, 0.8, 0.8};
const double strokeColorHighlighted[] = {1.0, 0.5, 0.5, 0.8};

// Scale widget params
const double scaleWidgetCornerSize = 8;
const double scaleWidgetEdgeSize = 5;
const double lineWidth = 1.0;

// Arrow params
const double rotateWidgetSize = scaleWidgetCornerSize;
const double rotateWidgetAngleRange = PI / 7;
const double rotateWidgetCircleCenter = 5;
const double rotateWidgetCircleRadius = 9;
const double rotateWidgetBodyHalfWidth = 0.7;
const double rotateWidgetHeadHalfWidth = SQRT2;
const int rotateWidgetNumSamples = 20;

}

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

void TransformTool::glPickColor_(WidgetId id) const
{
    Picking::glColor(idOffset_ + id - MIN_WIDGET_ID);
}

void TransformTool::drawScaleWidget_(double x, double y, double size, WidgetId id, ViewSettings & viewSettings) const
{
    size = size / viewSettings.zoom();

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

namespace
{

inline Vec2 u_(double theta)
{
    return Vec2(std::cos(theta), std::sin(theta));
}

inline Vec2 p_(const Vec2 & c, double r, double theta)
{
    return c + r * u_(theta);
}

inline Vec2 p_(const Vec2 & c, double r, const Vec2 & u)
{
    return c + r * u;
}

Vec2Vector rotateWidgetGeometry_(double x, double y, double midAngle, double size)
{
    // Returns a vector of points defining the arrow contour:
    //   - 3 points at the beginning for the first arrow head
    //   - 2*n points in the middle for the arrow body
    //   - points at the end for the second arrow head
    //
    // So 2*n + 6 points in total. See diagram below indicating
    // which indices correspond to which part of the arrow. Each '*'
    // is one point, and the number next to it is its index.
    //
    //                                              r (distance to circle center)
    //        0                     n+5             ^
    //          *    [2n+5..n+6]    *               | rMaxHead
    //    1     * * * * * * * * * * *               | rMaxBody
    //     *                             * n+4      | rCenterline
    //          * * * * * * * * * * *               | rMinBody
    //          *      [3..n+2]     *               | rMinHead
    //         2                     n+3            |

    // Vector to return
    const int & n = rotateWidgetNumSamples;
    Vec2Vector res(2*n+6);

    // Get circle center
    const Vec2 corner(x, y);
    const Vec2 center = p_(corner, -rotateWidgetCircleCenter*size, midAngle);

    // Get radiuses
    const double rCenterline = rotateWidgetCircleRadius*size;
    const double rMaxHead = rCenterline + rotateWidgetHeadHalfWidth*size;
    const double rMinHead = rCenterline - rotateWidgetHeadHalfWidth*size;
    const double rMaxBody = rCenterline + rotateWidgetBodyHalfWidth*size;
    const double rMinBody = rCenterline - rotateWidgetBodyHalfWidth*size;

    // Get angles
    const double startAngle = midAngle - 0.5 * rotateWidgetAngleRange;
    const double endAngle = midAngle + 0.5 * rotateWidgetAngleRange;
    const double deltaAngle = rotateWidgetAngleRange / (n-1);

    // First arrow head
    const Vec2 uStart = u_(startAngle);
    const Vec2 vStart(-uStart[1], uStart[0]);
    res[0] = p_(center, rMaxHead, uStart);
    res[1] = p_(center, rCenterline, uStart) - rotateWidgetHeadHalfWidth*size*vStart;
    res[2] = p_(center, rMinHead, uStart);

    // Second arrow head
    const Vec2 uEnd = u_(endAngle);
    const Vec2 vEnd(-uEnd[1], uEnd[0]);
    res[n+3] = p_(center, rMinHead, uEnd);
    res[n+4] = p_(center, rCenterline, uEnd) + rotateWidgetHeadHalfWidth*size*vEnd;
    res[n+5] = p_(center, rMaxHead, uEnd);

    // Arrow body
    int minBodyIndex = 3;
    int maxBodyIndex = 2*n+5;
    for (int i=0; i<n; ++i)
    {
        const Vec2 u = u_(startAngle + i * deltaAngle);

        res[minBodyIndex] = p_(center, rMinBody, u);
        res[maxBodyIndex] = p_(center, rMaxBody, u);

        ++minBodyIndex;
        --maxBodyIndex;
    }

    // Return
    return res;
}

}

void TransformTool::drawRotateWidget_(double x, double y, double midAngle,
                                      WidgetId id, ViewSettings & viewSettings) const
{
    const int & n = rotateWidgetNumSamples;
    const Vec2Vector arrow =
            rotateWidgetGeometry_(x, y, midAngle, rotateWidgetSize / viewSettings.zoom());


    // Fill
    if(hovered_ == id)
    {
        glColor4dv(fillColorHighlighted);
    }
    else
    {
        glColor4dv(fillColor);
    }
    // Arrow body
    glBegin(GL_TRIANGLE_STRIP);
    {
        int minBodyIndex = 3;
        int maxBodyIndex = 2*n+5;
        for (int i=0; i<n; ++i)
        {
            glVertex2d(arrow[minBodyIndex][0], arrow[minBodyIndex][1]);
            glVertex2d(arrow[maxBodyIndex][0], arrow[maxBodyIndex][1]);
            ++minBodyIndex;
            --maxBodyIndex;
        }
    }
    glEnd();
    // Arrow heads
    glBegin(GL_TRIANGLES);
    {
        glVertex2d(arrow[0][0], arrow[0][1]);
        glVertex2d(arrow[1][0], arrow[1][1]);
        glVertex2d(arrow[2][0], arrow[2][1]);
        glVertex2d(arrow[n+3][0], arrow[n+3][1]);
        glVertex2d(arrow[n+4][0], arrow[n+4][1]);
        glVertex2d(arrow[n+5][0], arrow[n+5][1]);
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
        for (unsigned int i=0; i<arrow.size(); ++i)
        {
            glVertex2d(arrow[i][0], arrow[i][1]);
        }
    }
    glEnd();
}

void TransformTool::drawPickRotateWidget_(double x, double y, double midAngle,
                                          WidgetId id, ViewSettings & viewSettings) const
{
    const int & n = rotateWidgetNumSamples;
    const Vec2Vector arrow =
            rotateWidgetGeometry_(x, y, midAngle, rotateWidgetSize / viewSettings.zoom());

    glPickColor_(id);

    // Arrow body
    glBegin(GL_TRIANGLE_STRIP);
    {
        int minBodyIndex = 3;
        int maxBodyIndex = 2*n+5;
        for (int i=0; i<n; ++i)
        {
            glVertex2d(arrow[minBodyIndex][0], arrow[minBodyIndex][1]);
            glVertex2d(arrow[maxBodyIndex][0], arrow[maxBodyIndex][1]);
            ++minBodyIndex;
            --maxBodyIndex;
        }
    }
    glEnd();

    // Arrow heads
    glBegin(GL_TRIANGLES);
    {
        glVertex2d(arrow[0][0], arrow[0][1]);
        glVertex2d(arrow[1][0], arrow[1][1]);
        glVertex2d(arrow[2][0], arrow[2][1]);
        glVertex2d(arrow[n+3][0], arrow[n+3][1]);
        glVertex2d(arrow[n+4][0], arrow[n+4][1]);
        glVertex2d(arrow[n+5][0], arrow[n+5][1]);
    }
    glEnd();
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

        // Rotate widgets
        drawRotateWidget_(bb.xMin(), bb.yMin(), 5*PI/4, TopLeftRotate, viewSettings);
        drawRotateWidget_(bb.xMax(), bb.yMin(), 7*PI/4, TopRightRotate, viewSettings);
        drawRotateWidget_(bb.xMax(), bb.yMax(), 1*PI/4, BottomRightRotate, viewSettings);
        drawRotateWidget_(bb.xMin(), bb.yMax(), 3*PI/4, BottomLeftRotate, viewSettings);
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

        // Rotate widgets
        drawPickRotateWidget_(bb.xMin(), bb.yMin(), 5*PI/4, TopLeftRotate, viewSettings);
        drawPickRotateWidget_(bb.xMax(), bb.yMin(), 7*PI/4, TopRightRotate, viewSettings);
        drawPickRotateWidget_(bb.xMax(), bb.yMax(), 1*PI/4, BottomRightRotate, viewSettings);
        drawPickRotateWidget_(bb.xMin(), bb.yMax(), 3*PI/4, BottomLeftRotate, viewSettings);
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
    // Clear cached values
    draggedVertices_.clear();
    draggedEdges_.clear();

    // Return in trivial cases
    if (hovered() == None || cells.isEmpty())
        return;

    // Keyframe inbetween cells
    CellSet cellsNotToKeyframe;
    CellSet cellsToKeyframe;
    foreach(Cell * c, cells)
    {
        InbetweenCell * sc = c->toInbetweenCell();
        if(sc)
        {
            if(sc->exists(time))
            {
                cellsToKeyframe << sc;
            }
            else
            {
                cellsNotToKeyframe << sc;
            }
        }
        else
        {
            cellsNotToKeyframe << c;
        }
    }
    VAC * vac = (*cells.begin())->vac();
    KeyCellSet keyframedCells = vac->keyframe_(cellsToKeyframe,time);

    // Determine which cells to transform
    CellSet cellsToTransform = cellsNotToKeyframe;
    foreach(KeyCell * c, keyframedCells)
        cellsToTransform << c;
    cellsToTransform = Algorithms::closure(cellsToTransform);

    // Cache key vertices and edges
    // XXX add the non-loop edges whose end vertices are dragged?
    draggedVertices_ = KeyVertexSet(cellsToTransform);
    draggedEdges_ = KeyEdgeSet(cellsToTransform);

    // prepare for affine transform
    foreach(KeyEdge * e, draggedEdges_)
        e->prepareAffineTransform();
    foreach(KeyVertex * v, draggedVertices_)
        v->prepareAffineTransform();

    // Compute selection bounding box at current time
    BoundingBox bb;
    for (CellSet::ConstIterator it = cells.begin(); it != cells.end(); ++it)
    {
        bb.unite((*it)->boundingBox(time));
    }

    // Cache start values to determine affine transformation
    x0_ = x0;
    y0_ = y0;
    if (hovered() == TopLeftScale ||
        hovered() == TopLeftRotate ||
        hovered() == TopScale ||
        hovered() == LeftScale)
    {
        xPivot_ = bb.xMax();
        yPivot_ = bb.yMax();
    }
    else if (hovered() == TopRightScale ||
             hovered() == TopRightRotate)
    {
        xPivot_ = bb.xMin();
        yPivot_ = bb.yMax();
    }
    else if (hovered() == BottomRightScale ||
             hovered() == BottomRightRotate ||
             hovered() == BottomScale ||
             hovered() == RightScale)
    {
        xPivot_ = bb.xMin();
        yPivot_ = bb.yMin();
    }
    else if (hovered() == BottomLeftScale ||
             hovered() == BottomLeftRotate)
    {
        xPivot_ = bb.xMax();
        yPivot_ = bb.yMin();
    }
}

void TransformTool::continueTransform(const CellSet & cells, double x, double y)
{
    // Return in trivial cases
    if (hovered() == None || cells.isEmpty())
        return;

    // Determine affine transformation
    Eigen::Affine2d xf;
    if (hovered() == TopLeftScale ||
        hovered() == TopRightScale ||
        hovered() == BottomRightScale ||
        hovered() == BottomLeftScale)
    {
        xf = Eigen::Scaling((x-xPivot_)/(x0_-xPivot_),
                            (y-yPivot_)/(y0_-yPivot_));
    }
    else if (hovered() == TopScale ||
             hovered() == BottomScale)
    {
        xf = Eigen::Scaling(1.0,
                            (y-yPivot_)/(y0_-yPivot_));
    }
    else if (hovered() == RightScale ||
             hovered() == LeftScale)
    {
        xf = Eigen::Scaling((x-xPivot_)/(x0_-xPivot_),
                            1.0);
    }
    else if (hovered() == TopLeftRotate ||
             hovered() == TopRightRotate ||
             hovered() == BottomRightRotate ||
             hovered() == BottomLeftRotate)
    {
        double theta0 = std::atan2(y0_ - yPivot_, x0_ - xPivot_);
        double theta  = std::atan2(y   - yPivot_, x   - xPivot_);
        double dTheta = theta - theta0;

        xf = Eigen::Rotation2Dd(dTheta);
    }
    else
    {
        return;
    }

    // Make relative to pivot
    Eigen::Translation2d pivot(xPivot_, yPivot_);
    xf = pivot * xf * pivot.inverse();

    // Apply affine transformation
    foreach(KeyEdge * e, draggedEdges_)
        e->performAffineTransform(xf);

    foreach(KeyVertex * v, draggedVertices_)
        v->performAffineTransform(xf);

    foreach(KeyVertex * v, draggedVertices_)
        v->correctEdgesGeometry();
}

void TransformTool::endTransform(const CellSet & /*cells*/)
{
    // Nothing to do
}

}
