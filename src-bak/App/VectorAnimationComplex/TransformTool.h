// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef TRANSFORMTOOL_H
#define TRANSFORMTOOL_H

#include <QObject>

#include <QSet>
#include "TimeDef.h"
#include "CellList.h"
#include "BoundingBox.h"

#include "Eigen.h"

class ViewSettings;

namespace VectorAnimationComplex
{

class TransformTool: public QObject
{
    Q_OBJECT

public:
    // Constructor
    TransformTool(QObject * parent = 0);

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
    void beginTransform(double x0, double y0, Time time);
    void continueTransform(double x, double y);
    void endTransform();

    // Drag and drop transform tool
    void prepareDragAndDrop();
    void performDragAndDrop(double dx, double dy);
    void endDragAndDrop();

private slots:
    void onKeyboardModifiersChanged();

private:
    // Disable copy and assignment
    TransformTool(const TransformTool &);
    TransformTool & operator=(const TransformTool &);

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
    bool useAltTransform_() const;
    bool isPivotCached_  () const;

    Eigen::Vector2d manualPivotPosition_         () const;
    Eigen::Vector2d cachedTransformPivotPosition_() const;
    Eigen::Vector2d cachedPivotPosition_         () const;

    Eigen::Vector2d computePivotPosition_(Time time) const;
    Eigen::Vector2d computePivotPosition_(const BoundingBox & bb) const;

    Eigen::Vector2d pivotPosition_           (const BoundingBox & bb) const;
    Eigen::Vector2d noTransformPivotPosition_(const BoundingBox & bb) const;

    Eigen::Vector2d transformPivotPosition_       (WidgetId id, const BoundingBox & bb) const;
    Eigen::Vector2d defaultTransformPivotPosition_(WidgetId id, const BoundingBox & bb) const;
    Eigen::Vector2d altTransformPivotPosition_    (WidgetId id, const BoundingBox & bb) const;

    bool manualPivot_;
    bool draggingManualPivot_;
    double xManualPivot_, yManualPivot_;
    double xManualPivot0_, yManualPivot0_;

    bool dragAndDropping_;
    bool transforming_;
    bool rotating_;
    double xTransformPivot_, yTransformPivot_;
    double xTransformPivotAlt_, yTransformPivotAlt_;

    // Apply transformation
    bool isTransformConstrained_() const;
    KeyVertexSet draggedVertices_;
    KeyEdgeSet draggedEdges_;
    double x0_, y0_, dx_, dy_, x_, y_;
    BoundingBox bb0_, obb0_;
    double dTheta_;
};

}

#endif // TRANSFORMTOOL_H
