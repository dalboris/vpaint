// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2012-2015 Boris Dalstein <dalboris@gmail.com>
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

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

class View;
class Scene;

class ViewMacOsX: public QWidget
{
    Q_OBJECT

public:
    ViewMacOsX(Scene * scene, QWidget *parent);
    View * view() const;

private:
    View * view_;
};

#endif // VIEWMACOSX_H
