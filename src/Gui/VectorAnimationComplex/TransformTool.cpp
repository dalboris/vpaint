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
#include "Global.h"

#include <cmath>
#include <vector>

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
const double outlineBoundingBoxColor[] = {0.5, 0.5, 0.5, 0.2};
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

// Pivot params
const double pivotWidgetSize = 0.5*scaleWidgetCornerSize;
const int pivotWidgetNumSamples = 20;

// Widget position
Vec2 widgetPos_(TransformTool::WidgetId id, const BoundingBox & bb)
{
    switch (id)
    {
    case TransformTool::TopLeftScale:      return Vec2(bb.xMin(), bb.yMin());
    case TransformTool::TopRightScale:     return Vec2(bb.xMax(), bb.yMin());
    case TransformTool::BottomRightScale:  return Vec2(bb.xMax(), bb.yMax());
    case TransformTool::BottomLeftScale:   return Vec2(bb.xMin(), bb.yMax());

    case TransformTool::TopScale:          return Vec2(bb.xMid(), bb.yMin());
    case TransformTool::RightScale:        return Vec2(bb.xMax(), bb.yMid());
    case TransformTool::BottomScale:       return Vec2(bb.xMid(), bb.yMax());
    case TransformTool::LeftScale:         return Vec2(bb.xMin(), bb.yMid());

    case TransformTool::TopLeftRotate:     return Vec2(bb.xMin(), bb.yMin());
    case TransformTool::TopRightRotate:    return Vec2(bb.xMax(), bb.yMin());
    case TransformTool::BottomRightRotate: return Vec2(bb.xMax(), bb.yMax());
    case TransformTool::BottomLeftRotate:  return Vec2(bb.xMin(), bb.yMax());

    case TransformTool::Pivot:             return Vec2(bb.xMid(), bb.yMid());

    // Silence warning
    default: return Vec2(0.0, 0.0);
    }
}

// Widget opposite position
Vec2 widgetOppositePos_(TransformTool::WidgetId id, const BoundingBox & bb)
{
    switch (id)
    {
    case TransformTool::TopLeftScale:      return Vec2(bb.xMax(), bb.yMax());
    case TransformTool::TopRightScale:     return Vec2(bb.xMin(), bb.yMax());
    case TransformTool::BottomRightScale:  return Vec2(bb.xMin(), bb.yMin());
    case TransformTool::BottomLeftScale:   return Vec2(bb.xMax(), bb.yMin());

    case TransformTool::TopScale:          return Vec2(bb.xMid(), bb.yMax());
    case TransformTool::RightScale:        return Vec2(bb.xMin(), bb.yMid());
    case TransformTool::BottomScale:       return Vec2(bb.xMid(), bb.yMin());
    case TransformTool::LeftScale:         return Vec2(bb.xMax(), bb.yMid());

    case TransformTool::TopLeftRotate:     return Vec2(bb.xMax(), bb.yMax());
    case TransformTool::TopRightRotate:    return Vec2(bb.xMin(), bb.yMax());
    case TransformTool::BottomRightRotate: return Vec2(bb.xMin(), bb.yMin());
    case TransformTool::BottomLeftRotate:  return Vec2(bb.xMax(), bb.yMin());

    case TransformTool::Pivot:             return Vec2(bb.xMid(), bb.yMid());

    // Silence warning
    default: return Vec2(0.0, 0.0);
    }
}

// Widget angle
double rotateWidgetMidAngle_(TransformTool::WidgetId id)
{
    switch (id)
    {
    case TransformTool::TopLeftRotate:     return 5*PI/4;
    case TransformTool::TopRightRotate:    return 7*PI/4;
    case TransformTool::BottomRightRotate: return 1*PI/4;
    case TransformTool::BottomLeftRotate:  return 3*PI/4;

    // Silence warning
    default: return 0.0;
    }
}

// Unit vector of angle theta
inline Vec2 u_(double theta)
{
    return Vec2(std::cos(theta), std::sin(theta));
}

// Point on circle of center c, radius r, at angle theta
inline Vec2 p_(const Vec2 & c, double r, double theta)
{
    return c + r * u_(theta);
}

// Point on circle of center c, radius r, along unit vector u
inline Vec2 p_(const Vec2 & c, double r, const Vec2 & u)
{
    return c + r * u;
}

// Compute arrow for the rotate widgets
Vec2Vector computeArrow_(TransformTool::WidgetId id, const BoundingBox & bb, ViewSettings & viewSettings)
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

    // Get circle parameters
    const Vec2   corner   = widgetPos_(id, bb);
    const double midAngle = rotateWidgetMidAngle_(id);
    const double size     = rotateWidgetSize / viewSettings.zoom();
    const Vec2   center   = p_(corner, -rotateWidgetCircleCenter*size, midAngle);

    // Get radiuses
    const double rCenterline = rotateWidgetCircleRadius*size;
    const double rMaxHead    = rCenterline + rotateWidgetHeadHalfWidth*size;
    const double rMinHead    = rCenterline - rotateWidgetHeadHalfWidth*size;
    const double rMaxBody    = rCenterline + rotateWidgetBodyHalfWidth*size;
    const double rMinBody    = rCenterline - rotateWidgetBodyHalfWidth*size;

    // Get angles
    const double startAngle = midAngle - 0.5 * rotateWidgetAngleRange;
    const double endAngle   = midAngle + 0.5 * rotateWidgetAngleRange;
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

void glStrokeBoundingBox_(const BoundingBox & bb)
{
    glBegin(GL_LINE_LOOP);
    {
        glVertex2d(bb.xMin(), bb.yMin());
        glVertex2d(bb.xMax(), bb.yMin());
        glVertex2d(bb.xMax(), bb.yMax());
        glVertex2d(bb.xMin(), bb.yMax());
    }
    glEnd();
}

void glStrokeRect_(const Vec2 & pos, double size)
{
    glBegin(GL_LINE_LOOP);
    {
        glVertex2d(pos[0] - size, pos[1] - size);
        glVertex2d(pos[0] + size, pos[1] - size);
        glVertex2d(pos[0] + size, pos[1] + size);
        glVertex2d(pos[0] - size, pos[1] + size);
    }
    glEnd();
}

void glFillRect_(const Vec2 & pos, double size)
{
    glBegin(GL_QUADS);
    {
        glVertex2d(pos[0] - size, pos[1] - size);
        glVertex2d(pos[0] + size, pos[1] - size);
        glVertex2d(pos[0] + size, pos[1] + size);
        glVertex2d(pos[0] - size, pos[1] + size);
    }
    glEnd();
}

void glStrokeArrow_(const Vec2Vector & arrow)
{
    glBegin(GL_LINE_LOOP);
    {
        for (unsigned int i=0; i<arrow.size(); ++i)
        {
            glVertex2d(arrow[i][0], arrow[i][1]);
        }
    }
    glEnd();
}

void glFillArrow_(const Vec2Vector & arrow)
{
    const int & n = rotateWidgetNumSamples;

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

void glStrokePivot_(const Vec2 & pos, double size)
{
    glBegin(GL_LINE_LOOP);
    {
        const int & n = pivotWidgetNumSamples;
        for (int i=0; i<n; ++i)
        {
            const Vec2 p = p_(pos, size, 2*i*PI/n);
            glVertex2d(p[0], p[1]);
        }
    }
    glEnd();

    glBegin(GL_LINES);
    {
        glVertex2d(pos[0] - 2*size, pos[1]);
        glVertex2d(pos[0] + 2*size, pos[1]);
        glVertex2d(pos[0], pos[1] - 2*size);
        glVertex2d(pos[0], pos[1] + 2*size);
    }
    glEnd();
}

void glFillPivot_(const Vec2 & pos, double size)
{
    glBegin(GL_POLYGON);
    {
        const int & n = pivotWidgetNumSamples;
        for (int i=0; i<n; ++i)
        {
            const Vec2 p = p_(pos, size, 2*i*PI/n);
            glVertex2d(p[0], p[1]);
        }
    }
    glEnd();
}

}

TransformTool::TransformTool(QObject * parent) :
    QObject(parent),

    cells_(),
    idOffset_(0),
    hovered_(None),
    manualPivot_(false),
    transformPivot_(false)
{
    connect(global(), SIGNAL(keyboardModifiersChanged()), this, SLOT(onKeyboardModifiersChanged()));
}

void TransformTool::setCells(const CellSet & cells)
{
    cells_ = cells;
    manualPivot_ = false;
    transformPivot_ = false;

    // Note: we can't pre-compute bounding boxes or pivot position here
    //       since we don't know the time.
}

void TransformTool::setIdOffset(int idOffset)
{
    idOffset_ = idOffset;
}

TransformTool::WidgetId TransformTool::hovered() const
{
    return hovered_;
}

Eigen::Vector2d TransformTool::pivotPosition(Time time) const
{
    if (isPivotPrecomputed_())
    {
        return precomputedPivotPosition_();
    }
    else
    {
        return computePivotPosition_(time);
    }
}

Eigen::Vector2d TransformTool::pivotPosition_(const BoundingBox & bb) const
{
    if (isPivotPrecomputed_())
    {
        return precomputedPivotPosition_();
    }
    else
    {
        return computePivotPosition_(bb);
    }
}

Eigen::Vector2d TransformTool::computePivotPosition_(Time time) const
{
    // Compute outline bounding box at current time
    BoundingBox obb;
    for (CellSet::ConstIterator it = cells_.begin(); it != cells_.end(); ++it)
    {
        obb.unite((*it)->outlineBoundingBox(time));
    }

    // Compute pivot position from bounding box
    return computePivotPosition_(obb);
}

Eigen::Vector2d TransformTool::computePivotPosition_(const BoundingBox & bb) const
{
    return widgetPos_(Pivot, bb);
}

bool TransformTool::isPivotPrecomputed_() const
{
    return transformPivot_ || manualPivot_;
}

Eigen::Vector2d TransformTool::precomputedPivotPosition_() const
{
    if (transformPivot_)
    {
        if (global()->keyboardModifiers().testFlag(Qt::AltModifier))
        {
            return Vec2(xTransformPivotAlt_, yTransformPivotAlt_);
        }
        else
        {
            return Vec2(xTransformPivot_, yTransformPivot_);
        }
    }
    else if (manualPivot_)
    {
        return Vec2(xManualPivot_, yManualPivot_);
    }
    else
    {
        qDebug("Warning: calling precomputedPivotPosition_() while pivot is not precomputed.");
        return Eigen::Vector2d(0.0, 0.0);
    }
}

void TransformTool::glFillColor_(WidgetId id) const
{
    glColor4dv(hovered_ == id ? fillColorHighlighted : fillColor);
}

void TransformTool::glStrokeColor_(WidgetId id) const
{
    glColor4dv(hovered_ == id ? strokeColorHighlighted : strokeColor);
}

void TransformTool::glPickColor_(WidgetId id) const
{
    Picking::glColor(idOffset_ + id - MIN_WIDGET_ID);
}

void TransformTool::drawScaleWidget_(WidgetId id, const BoundingBox & bb,
                                     double size, ViewSettings & viewSettings) const
{
    // Compute rect
    const Vec2 pos = widgetPos_(id, bb);
    size = size / viewSettings.zoom();

    // Fill
    glFillColor_(id);
    glFillRect_(pos, size);

    // Stroke
    glStrokeColor_(id);
    glStrokeRect_(pos, size);
}

void TransformTool::drawPickScaleWidget_(WidgetId id, const BoundingBox & bb,
                                         double size, ViewSettings &viewSettings) const
{
    // Compute pos and size
    const Vec2 pos = widgetPos_(id, bb);
    size = size / viewSettings.zoom();

    // Fill
    glPickColor_(id);
    glFillRect_(pos, size);
}

void TransformTool::drawRotateWidget_(WidgetId id, const BoundingBox & bb,
                                      ViewSettings & viewSettings) const
{
    // Compute arrow
    const Vec2Vector arrow = computeArrow_(id, bb, viewSettings);

    // Fill
    glFillColor_(id);
    glFillArrow_(arrow);

    // Stroke
    glStrokeColor_(id);
    glStrokeArrow_(arrow);
}

void TransformTool::drawPickRotateWidget_(WidgetId id, const BoundingBox & bb,
                                          ViewSettings & viewSettings) const
{
    // Compute arrow
    const Vec2Vector arrow = computeArrow_(id, bb, viewSettings);

    // Fill
    glPickColor_(id);
    glFillArrow_(arrow);
}

void TransformTool::drawPivot_(const BoundingBox & bb, ViewSettings & viewSettings) const
{
    // Compute pos and size
    const Vec2 pos = pivotPosition_(bb);
    const double size = pivotWidgetSize / viewSettings.zoom();

    // Fill
    glFillColor_(Pivot);
    glFillPivot_(pos, size);

    // Stroke
    glStrokeColor_(Pivot);
    glStrokePivot_(pos, size);
}

void TransformTool::drawPickPivot_(const BoundingBox & bb, ViewSettings & viewSettings) const
{
    // Compute pos and size
    const Vec2 pos = pivotPosition_(bb);
    const double size = pivotWidgetSize / viewSettings.zoom();

    // Fill
    glPickColor_(Pivot);
    glFillPivot_(pos, size);
}

void TransformTool::draw(const CellSet & cells, Time time, ViewSettings & viewSettings) const
{
    // Compute bounding box at current time
    BoundingBox bb;
    for (CellSet::ConstIterator it = cells.begin(); it != cells.end(); ++it)
    {
        bb.unite((*it)->boundingBox(time));
    }

    // Compute outline bounding box at current time
    BoundingBox obb;
    for (CellSet::ConstIterator it = cells.begin(); it != cells.end(); ++it)
    {
        obb.unite((*it)->outlineBoundingBox(time));
    }

    // Draw bounding box and transform widgets
    if (bb.isProper())
    {
        glLineWidth(lineWidth);

        // Outline bounding box
        glColor4dv(outlineBoundingBoxColor);
        glStrokeBoundingBox_(obb);

        // Bounding box
        glColor4dv(boundingBoxColor);
        glStrokeBoundingBox_(bb);

        // Scale widgets (corners)
        for (WidgetId id: {TopLeftScale, TopRightScale, BottomRightScale, BottomLeftScale})
            drawScaleWidget_(id, bb, scaleWidgetCornerSize, viewSettings);

        // Scale widgets (edges)
        for (WidgetId id: {TopScale, RightScale, BottomScale, LeftScale})
            drawScaleWidget_(id, bb, scaleWidgetEdgeSize, viewSettings);

        // Rotate widgets
        for (WidgetId id: {TopLeftRotate, TopRightRotate, BottomRightRotate, BottomLeftRotate})
            drawRotateWidget_(id, bb, viewSettings);

        // Pivot
        drawPivot_(obb, viewSettings);
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

    // Compute outline bounding box at current time
    BoundingBox obb;
    for (CellSet::ConstIterator it = cells.begin(); it != cells.end(); ++it)
    {
        obb.unite((*it)->outlineBoundingBox(time));
    }

    // Draw transform widgets
    if (bb.isProper())
    {
        // Scale widgets (corners)
        for (WidgetId id: {TopLeftScale, TopRightScale, BottomRightScale, BottomLeftScale})
            drawPickScaleWidget_(id, bb, scaleWidgetCornerSize, viewSettings);

        // Scale widgets (edges)
        for (WidgetId id: {TopScale, RightScale, BottomScale, LeftScale})
            drawPickScaleWidget_(id, bb, scaleWidgetEdgeSize, viewSettings);

        // Rotate widgets
        for (WidgetId id: {TopLeftRotate, TopRightRotate, BottomRightRotate, BottomLeftRotate})
            drawPickRotateWidget_(id, bb, viewSettings);

        // Pivot
        drawPickPivot_(obb, viewSettings);
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

void TransformTool::beginTransform(double x0, double y0, Time time)
{
    // Clear cached values
    draggedVertices_.clear();
    draggedEdges_.clear();

    // Return in trivial cases
    if (hovered() == None || cells_.isEmpty())
        return;

    // Move pivot
    if (hovered() == Pivot)
    {
        // Cache initial pivot position
        const Vec2 pivotPos = pivotPosition(time);
        xManualPivot0_ = pivotPos[0];
        yManualPivot0_ = pivotPos[1];

        // Cache initial mouse position
        x0_ = x0;
        y0_ = y0;
    }

    // Transform selection
    else
    {
        // Compute outline bounding box at current time
        BoundingBox obb;
        for (CellSet::ConstIterator it = cells_.begin(); it != cells_.end(); ++it)
        {
            obb.unite((*it)->outlineBoundingBox(time));
        }

        // Keyframe inbetween cells
        CellSet cellsNotToKeyframe;
        CellSet cellsToKeyframe;
        foreach(Cell * c, cells_)
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
        VAC * vac = (*cells_.begin())->vac();
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

        // Cache start values to determine affine transformation:
        //   - x0_, y0_: start mouse position
        //   - dx_, dy_: offset between mouse position and perfect position on obb
        //   - xPivot_, yPivot_: position of the pivot point

        const Vec2 currentPivotPos = pivotPosition_(obb);
        const Vec2 obbWidgetPos = widgetPos_(hovered(), obb);
        const Vec2 obbOppositeWidgetPos = widgetOppositePos_(hovered(), obb);

        x0_ = x0;
        y0_ = y0;

        dx_ = x0 - obbWidgetPos[0];
        dy_ = y0 - obbWidgetPos[1];

        // Cache current manual pivot position
        xManualPivot0_ = currentPivotPos[0];
        yManualPivot0_ = currentPivotPos[1];

        // Set default and alternative transform pivot position
        if (hovered() == TopLeftRotate ||
            hovered() == TopRightRotate ||
            hovered() == BottomRightRotate ||
            hovered() == BottomLeftRotate)
        {
            xTransformPivot_ = currentPivotPos[0];
            yTransformPivot_ = currentPivotPos[1];
            xTransformPivotAlt_ = obbOppositeWidgetPos[0];
            yTransformPivotAlt_ = obbOppositeWidgetPos[1];
        }
        else
        {
            xTransformPivot_ = obbOppositeWidgetPos[0];
            yTransformPivot_ = obbOppositeWidgetPos[1];
            xTransformPivotAlt_ = currentPivotPos[0];
            yTransformPivotAlt_ = currentPivotPos[1];
        }
    }
}

void TransformTool::continueTransform(double x, double y)
{
    // Cache values
    x_ = x;
    y_ = y;

    // Return in trivial cases
    if (hovered() == None || cells_.isEmpty())
        return;

    // Move pivot
    if (hovered() == Pivot)
    {
        manualPivot_ = true;
        xManualPivot_ = xManualPivot0_ + x - x0_;
        yManualPivot_ = yManualPivot0_ + y - y0_;
    }

    // Transform selection
    else
    {
        // Inform that we are currently transforming the selection
        transformPivot_ = true;

        // Get pivot
        const Vec2 pivotPos = precomputedPivotPosition_();
        const double xPivot = pivotPos[0];
        const double yPivot = pivotPos[1];

        // Determine affine transformation
        Eigen::Affine2d xf;
        if (hovered() == TopLeftScale ||
            hovered() == TopRightScale ||
            hovered() == BottomRightScale ||
            hovered() == BottomLeftScale)
        {
            xf = Eigen::Scaling((x-dx_-xPivot)/(x0_-dx_-xPivot),
                                (y-dy_-yPivot)/(y0_-dy_-yPivot));
        }
        else if (hovered() == TopScale ||
                 hovered() == BottomScale)
        {
            xf = Eigen::Scaling(1.0,
                                (y-dy_-yPivot)/(y0_-dy_-yPivot));
        }
        else if (hovered() == RightScale ||
                 hovered() == LeftScale)
        {
            xf = Eigen::Scaling((x-dx_-xPivot)/(x0_-dx_-xPivot),
                                1.0);
        }
        else if (hovered() == TopLeftRotate ||
                 hovered() == TopRightRotate ||
                 hovered() == BottomRightRotate ||
                 hovered() == BottomLeftRotate)
        {
            double theta0 = std::atan2(y0_ - yPivot, x0_ - xPivot);
            double theta  = std::atan2(y   - yPivot, x   - xPivot);
            double dTheta = theta - theta0;

            xf = Eigen::Rotation2Dd(dTheta);
        }
        else
        {
            return;
        }

        // Make pivot point invariant by transformation
        Eigen::Translation2d pivot(xPivot, yPivot);
        xf = pivot * xf * pivot.inverse();

        // Apply affine transformation
        foreach(KeyEdge * e, draggedEdges_)
            e->performAffineTransform(xf);

        foreach(KeyVertex * v, draggedVertices_)
            v->performAffineTransform(xf);

        foreach(KeyVertex * v, draggedVertices_)
            v->correctEdgesGeometry();

        // Apply transformation to manual pivot point
        if (manualPivot_)
        {
            Vec2 manualPivot = xf * Vec2(xManualPivot0_, yManualPivot0_);
            xManualPivot_ = manualPivot[0];
            yManualPivot_ = manualPivot[1];
        }
    }
}

void TransformTool::endTransform()
{
    transformPivot_ = false;
}

void TransformTool::prepareDragAndDrop()
{
    xManualPivot0_ = xManualPivot_;
    yManualPivot0_ = yManualPivot_;
}

void TransformTool::performDragAndDrop(double dx, double dy)
{
    xManualPivot_ = xManualPivot0_ + dx;
    yManualPivot_ = yManualPivot0_ + dy;
}

void TransformTool::onKeyboardModifiersChanged()
{
    if (transformPivot_)
    {
        continueTransform(x_, y_);
    }
}

}
