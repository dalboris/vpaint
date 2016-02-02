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

class ViewSettings;

namespace VectorAnimationComplex
{

class TransformTool
{
public:
    // Constructor
    TransformTool();
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

        MIN_WIDGET_ID = TopLeftScale,
        MAX_WIDGET_ID = BottomLeftRotate
    };

    // Get which widget is currently hovered, if any
    WidgetId hovered() const;

    // Drawing
    void draw(const CellSet & cells, Time time, ViewSettings & viewSettings) const;

    // Picking
    void drawPick(const CellSet & cells, Time time, ViewSettings & viewSettings) const;
    void setHoveredObject(int id);
    void setNoHoveredObject();

    // Perform operation
    void beginTransform(const CellSet & cells, double x0, double y0, Time time);
    void continueTransform(const CellSet & cells, double x, double y);
    void endTransform(const CellSet & cells);

private:
    int idOffset_;
    WidgetId hovered_;

    void glPickColor_(WidgetId id) const;
};

}

#endif // TRANSFORMTOOL_H
