// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#ifndef VIEW2DMOUSEEVENT_H
#define VIEW2DMOUSEEVENT_H

#include "ViewMouseEvent.h"

/// \class View2DMouseEvent
/// \brief A suclass of ViewMouseEvent for View2D
///
/// In addition to the attributes provided by ViewMouseEvent, this class
/// provides the 2D mouse position in scene coordinates.
///
class View2DMouseEvent: public ViewMouseEvent
{
public:
    QPointF scenePos() const;        ///< Returns the current mouse position, in view coordinates.
    QPointF scenePosAtPress() const; ///< Returns the mouse position at mouse press, in view coordinates.

private:
    void computeSceneAttributes();
    void computeSceneAttributesAtPress();
    QPointF computeScenePos_(const QPointF & viewPos);

    QPointF scenePos_ = QPointF(0.0, 0.0);
    QPointF scenePosAtPress_ = QPointF(0.0, 0.0);
};

#endif // VIEW2DMOUSEEVENT_H
