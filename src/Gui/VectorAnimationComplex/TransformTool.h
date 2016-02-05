// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef TRANSFORMTOOL_H
#define TRANSFORMTOOL_H

#include <QSet>
#include "TimeDef.h"
#include "CellList.h"

#include "Eigen.h"

class ViewSettings;

namespace VectorAnimationComplex
{

class BoundingBox;

class TransformTool
{
public:
    // Constructor
    TransformTool();

    // Disable copy and assignment
    TransformTool(const TransformTool &) = delete;
    TransformTool & operator=(const TransformTool &) = delete;

    // Set state
    void setCells(const CellSet & cells);
    void setIdOffset(int idOffset);

    // Enum of all transform widgets IDs
    enum WidgetId
    {
        None,

        TopLeftScale,
        TopRightScale,
        BottomRightScale,
        BottomLeftScale,

        TopScale,
        RightScale,
        BottomScale,
        LeftScale,

        TopLeftRotate,
        TopRightRotate,
        BottomRightRotate,
        BottomLeftRotate,

        Pivot,

        MIN_WIDGET_ID = TopLeftScale,
        MAX_WIDGET_ID = Pivot
    };

    // Get which widget is currently hovered, if any
    WidgetId hovered() const;

    // Get pivot position
    Eigen::Vector2d pivotPosition(Time time) const;

    // Drawing
    void draw(const CellSet & cells, Time time, ViewSettings & viewSettings) const;

    // Picking
    void drawPick(const CellSet & cells, Time time, ViewSettings & viewSettings) const;
    void setHoveredObject(int id);
    void setNoHoveredObject();

    // Transform selection
    void beginTransform(const CellSet & cells, double x0, double y0, Time time);
    void continueTransform(const CellSet & cells, double x, double y);
    void endTransform(const CellSet & cells);

    // Drag and drop transform tool
    void prepareDragAndDrop();
    void performDragAndDrop(double dx, double dy);

private:

    CellSet cells_;
    int idOffset_;
    WidgetId hovered_;

    void glFillColor_(WidgetId id) const;
    void glStrokeColor_(WidgetId id) const;
    void glPickColor_(WidgetId id) const;

    void drawScaleWidget_(WidgetId id, const BoundingBox & bb, double size, ViewSettings & viewSettings) const;
    void drawPickScaleWidget_(WidgetId id, const BoundingBox & bb, double size, ViewSettings & viewSettings) const;

    void drawRotateWidget_(WidgetId id, const BoundingBox & bb, ViewSettings & viewSettings) const;
    void drawPickRotateWidget_(WidgetId id, const BoundingBox & bb, ViewSettings & viewSettings) const;

    void drawPivot_(const BoundingBox & bb, ViewSettings & viewSettings) const;
    void drawPickPivot_(const BoundingBox & bb, ViewSettings & viewSettings) const;

    // Pivot
    bool isPivotPrecomputed_() const;
    Eigen::Vector2d precomputedPivotPosition_() const;
    Eigen::Vector2d computePivotPosition_(Time time) const;
    Eigen::Vector2d computePivotPosition_(const BoundingBox & bb) const;
    Eigen::Vector2d pivotPosition_(const BoundingBox & bb) const;
    bool manualPivot_;
    double xManualPivot_, yManualPivot_;
    double xManualPivot0_, yManualPivot0_;
    bool transformPivot_;
    double xTransformPivot_, yTransformPivot_;
    double xTransformPivotAlt_, yTransformPivotAlt_;

    // Affine transform cached info
    KeyVertexSet draggedVertices_;
    KeyEdgeSet draggedEdges_;
    double x0_, y0_, dx_, dy_;
};

}

#endif // TRANSFORMTOOL_H
