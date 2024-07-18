// Copyright (C) 2012-2019 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Include this file if you need to call OpenGL functions. Assuming that
// a valid OpenGL context is current, here is how to call OpenGL functions.
//
// OpenGL 1.x functions can be directly called as global functions.
//
// OpenGL 2.x functions can be called via the following:
//
//   auto* f = getOpenGLFunctions(context());
//   f->glBlendFuncSeparate(...)
//
// If compiling with Qt 5, we use OpenGL 2.1. This means that OpenGL 3.x or 4.x
// are not direcly available, but may be available as extensions to OpenGL 2.1,
// for example:
//
//   // Query extensions
//   QList extensions = context()->extensions().toList();
//   std::sort(extensions);
//   qDebug() << "Supported extensions (" << extensions.count() << ")";
//   foreach (const QByteArray &extension, extensions)
//       qDebug() << "    " << extension;
//
//   // Check if extension is supported
//   if (!context()->hasExtension(QByteArrayLiteral(
//            "GL_ARB_instanced_arrays"))
//       qFatal("GL_ARB_instanced_arrays is not supported");
//
//   // Create instance of helper class and resolve functions
//   QOpenGLExtension_ARB_instanced_arrays* m_instanceFuncs =
//       new QOpenGLExtension_ARB_instanced_arrays();
//   m_instanceFuncs->initializeOpenGLFunctions();
//
//   // Call an extension function
//   m_instanceFuncs->glVertexAttribDivisorARB(pointLocation, 1);
//
// For more details, see: https://www.kdab.com/opengl-in-qt-5-1-part-1/
//
// If compiling with Qt 6, we use OpenGL 3.0, and extensions are not supported.

#ifndef OPENGL_H
#define OPENGL_H

#include <QtDebug>
#include <QtGlobal>
#include <QtOpenGL>
#include <QSurfaceFormat>

// VPaint uses legacy OpenGL and requires either:
// - OpenGL 2.1 with GL_ARB_framebuffer_object extension, or
// - OpenGL 3.0 and 3.1 with deprecated function
// - OpenGL 3+ with compatibility profile
//
// Unfortunately:
// - macOS only supports OpenGL 2.1, or OpenGL 3.2+ with core profile
// - Qt 6 removed the QtOpenGLExtensions module
// - Qt 5 is not natively supported on Apple Silicon (M1, ...)
//
// So in order to compile VPaint natively on macOS on Apple Silicon, we need:
// - Qt 6
// - OpenGL 2.1
// - Manually add back QtOpenGLExtensions.
//
// This is why we have manually added the Third/QtOpenGLExtensions folder

#define VPAINT_OPENGL_USE_2_1_WITH_EXTENSION 1

#if VPAINT_OPENGL_USE_2_1_WITH_EXTENSION

#include <QOpenGLFunctions_2_1>

#define VPAINT_OPENGL_VERSION_MAJOR 2
#define VPAINT_OPENGL_VERSION_MINOR 1
#define VPAINT_OPENGL_VERSION "2.1"

using OpenGLFunctions = QOpenGLFunctions_2_1;

static constexpr QSurfaceFormat::OpenGLContextProfile openGLProfile = QSurfaceFormat::NoProfile;
static constexpr QSurfaceFormat::FormatOptions openGLOptions = {};

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QOpenGLExtensions>
#else
#include <QtOpenGLExtensions/qopenglextensions.h>
#endif

using FrameBufferObjectPtr = std::unique_ptr<QOpenGLExtension_ARB_framebuffer_object>;

#else

#include <QOpenGLFunctions_3_2_Compatibility>

#define VPAINT_OPENGL_VERSION_MAJOR 3
#define VPAINT_OPENGL_VERSION_MINOR 2
#define VPAINT_OPENGL_VERSION "3.2"

using OpenGLFunctions = QOpenGLFunctions_3_2_Compatibility;

static constexpr QSurfaceFormat::OpenGLContextProfile openGLProfile = QSurfaceFormat::CompatibilityProfile;
static constexpr QSurfaceFormat::FormatOptions openGLOptions = QSurfaceFormat::DeprecatedFunctions;

using FrameBufferObjectPtr = OpenGLFunctions*;

#endif

inline OpenGLFunctions* getOpenGLFunctions(QOpenGLContext * context) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    OpenGLFunctions* gl = context->versionFunctions<OpenGLFunctions>();
#else
    OpenGLFunctions* gl = QOpenGLVersionFunctionsFactory::get<OpenGLFunctions>(context);
#endif
    if (!gl) {
        qFatal("Failed to access OpenGL " VPAINT_OPENGL_VERSION " functions.");
    }
    return gl;
}

inline void initFrameBufferObject(FrameBufferObjectPtr & fbo, QOpenGLContext * context) {

    if (fbo) {
        return;
    }

#if VPAINT_OPENGL_USE_2_1_WITH_EXTENSION

    // Query extensions
    bool queryExtensions = false;
    if (queryExtensions) {
        QList<QByteArray> extensions = context->extensions().values();
        qDebug() << "Supported extensions (" << extensions.count() << ")";
        foreach (const QByteArray &extension, extensions)
            qDebug() << "    " << extension;
    }

    // Access GL_ARB_framebuffer_object extension
    if (!context->hasExtension(QByteArrayLiteral("GL_ARB_framebuffer_object"))) {
        qFatal("GL_ARB_framebuffer_object is not supported");
    }
    fbo.reset(new QOpenGLExtension_ARB_framebuffer_object());
    fbo->initializeOpenGLFunctions();
#else
    fbo = getOpenGLFunctions(context);
#endif
}

#endif
