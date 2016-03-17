// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef VIEWMOUSEEVENT_H
#define VIEWMOUSEEVENT_H

#include <QMouseEvent>

/// \class ViewMouseEvent
/// \brief A class to store information about a View mouse event
///
/// Notes:
///   - Does not support multi-button presses.
///   - view coordinates = widget coordinates
///
struct ViewMouseEvent
{
    Qt::MouseButton button;          ///< which button initiated this
    Qt::KeyboardModifiers modifiers; ///< which modifiers were down at mouse press

    QPointF viewPos;        ///< current mouse position, in view coordinates
    QPointF viewPosAtPress; ///< mouse position at mouse press, in view coordinates

    QPointF scenePos;        ///< current mouse position, in scene coordinates
    QPointF scenePosAtPress; ///< mouse position at mouse press, in scene coordinates

    bool isTablet;                ///< is this generated from a tablet event?
    double tabletPressure;        ///< if tablet event, what is the current tablet pressure?
    double tabletPressureAtPress; ///< if tablet event, what was the tablet pressure at press?
};

#endif // VIEWMOUSEEVENT_H
