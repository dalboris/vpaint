// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

// An ad-hoc class to hold a View + ViewSettings, to workaround the limitation:
//   On Mac OS X, when Qt is built with Cocoa support, a QGLWidget can't
//   have any sibling widgets placed ontop of itself. This is due to limitations
//   in the Cocoa API and is not supported by Apple.
// (cf Qt doc, QGLWidget class)
//
// Therefore, on Mac, the ViewSettings do not lie on top of the view, but as a "toolbar"

#ifndef VIEWMACOSX_H
#define VIEWMACOSX_H

#include <QWidget>

class ViewOld;
class SceneOld;

class ViewMacOsX: public QWidget
{
    Q_OBJECT

public:
    ViewMacOsX(SceneOld * scene, QWidget *parent);
    ViewOld * view() const;

private:
    ViewOld * view_;
};

#endif // VIEWMACOSX_H
