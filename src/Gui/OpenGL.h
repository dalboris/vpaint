// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

// Include this file if you need to call OpenGL functions.
//
// OpenGL 2.1 functions can be directly called as global functions, assuming that
// a valid OpenGL context is current.
//
// OpenGL 3.0+ functions must be called from an OpenGLFunctions instance, which
// you can obtain via a QOpenGLContext:
//
//   auto* f = context()->versionFunctions<OpenGLFunctions>();
//   f->glBlendFuncSeparate(...)
//

#ifndef OPENGL_H
#define OPENGL_H

#include <QOpenGLFunctions_3_0>

using OpenGLFunctions = QOpenGLFunctions_3_0;

#define VPAINT_OPENGL_VERSION_MAJOR 3
#define VPAINT_OPENGL_VERSION_MINOR 0

#endif
