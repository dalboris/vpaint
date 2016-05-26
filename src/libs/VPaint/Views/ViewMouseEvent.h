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

class View;

/// \class ViewMouseEvent
/// \brief A class to store information about a View mouse event
///
/// Quick notes:
///   - Does not support multi-button presses.
///   - view coordinates = widget coordinates
///   - Setters are meant to be called by View
///   - Getters are meant to be called by ViewAction
///
class ViewMouseEvent
{
public:
    // Virtual destructor
    ~ViewMouseEvent() {}

    // Getters
    View * view() const; ///< Returns the view from which this mouse event is generated.

    Qt::MouseButton button() const;          ///< Returns which button initiated this mouse event.
    Qt::KeyboardModifiers modifiers() const; ///< Returns which modifiers were down at mouse press.

    QPointF viewPos() const;        ///< Returns the current mouse position, in view coordinates.
    QPointF viewPosAtPress() const; ///< Returns the mouse position at mouse press, in view coordinates.

    double timeSincePress() const; ///< Returns the time, in seconds, that has passed since the mouse press.

    bool isTablet() const;                ///< Returns whether this event was generated from a tablet event.
    double tabletPressure() const;        ///< If tablet event, returns the current tablet pressure.
    double tabletPressureAtPress() const; ///< If tablet event, returns the tablet pressure at press.

    // Setters
    void setView(View * view); ///< Sets the view from which this mouse event is generated.

    void setButton(Qt::MouseButton button);             ///< Sets which button initiated this mouse event.
    void setModifiers(Qt::KeyboardModifiers modifiers); ///< Sets which modifiers were down at mouse press.

    void setViewPos(const QPointF & viewPos);        ///< Sets current mouse position, in view coordinates.
    void setViewPosAtPress(const QPointF & viewPos); ///< Sets mouse position at mouse press, in view coordinates.

    void setTimeSincePress(double time); ///< Sets the time, in seconds, that has passed since the mouse press.

    void setTablet(bool isTablet);                      ///< Sets whether this event was generated from a tablet event.
    void setTabletPressure(double tabletPressure);        ///< If tablet event, sets the current tablet pressure.
    void setTabletPressureAtPress(double tabletPressure); ///< If tablet event, sets the tablet pressure at press.

protected:
    virtual void computeSceneAttributes() {}        ///< To be implemented in subclasses, invoked by setViewPos().
    virtual void computeSceneAttributesAtPress() {} ///< To be implemented in subclasses, invoked by setViewPosAtPress().

private:
    View * view_ = nullptr;

    Qt::MouseButton button_ = Qt::NoButton;
    Qt::KeyboardModifiers modifiers_ = Qt::NoModifier;

    QPointF viewPos_ = QPointF(0.0, 0.0);
    QPointF viewPosAtPress_ = QPointF(0.0, 0.0);

    double timeSincePress_ = 0.0;

    bool isTablet_ = false;
    double tabletPressure_ = 0.0;
    double tabletPressureAtPress_ = 0.0;
};

#endif // VIEWMOUSEEVENT_H
