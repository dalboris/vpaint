// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "ViewMouseEvent.h"

View * ViewMouseEvent::view() const
{
    return view_;
}

Qt::MouseButton ViewMouseEvent::button() const
{
    return button_;
}

Qt::KeyboardModifiers ViewMouseEvent::modifiers() const
{
    return modifiers_;
}

QPointF ViewMouseEvent::viewPos() const
{
    return viewPos_;
}

QPointF ViewMouseEvent::viewPosAtPress() const
{
    return viewPosAtPress_;
}

double ViewMouseEvent::timeSincePress() const
{
    return timeSincePress_;
}

bool ViewMouseEvent::isTablet() const
{
    return isTablet_;
}

double ViewMouseEvent::tabletPressure() const
{
    return tabletPressure_;
}

double ViewMouseEvent::tabletPressureAtPress() const
{
    return tabletPressureAtPress_;
}

void ViewMouseEvent::setView(View * view)
{
    view_ = view;
}

void ViewMouseEvent::setButton(Qt::MouseButton button)
{
    button_ = button;
}

void ViewMouseEvent::setModifiers(Qt::KeyboardModifiers modifiers)
{
    modifiers_ = modifiers;
}

void ViewMouseEvent::setViewPos(const QPointF & viewPos)
{
    viewPos_ = viewPos;
    computeSceneAttributes();
}

void ViewMouseEvent::setViewPosAtPress(const QPointF & viewPos)
{
    viewPosAtPress_ = viewPos;
    computeSceneAttributesAtPress();
}

void ViewMouseEvent::setTimeSincePress(double time)
{
    timeSincePress_ = time;
}

void ViewMouseEvent::setTablet(bool isTablet)
{
    isTablet_ = isTablet;
}

void ViewMouseEvent::setTabletPressure(double tabletPressure)
{
    tabletPressure_ = tabletPressure;
}

void ViewMouseEvent::setTabletPressureAtPress(double tabletPressure)
{
    tabletPressureAtPress_ = tabletPressure;
}
