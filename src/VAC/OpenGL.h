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

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)

#include <QOpenGLExtensions>
#include <QOpenGLFunctions_2_1>

#define VPAINT_OPENGL_VERSION_MAJOR 2
#define VPAINT_OPENGL_VERSION_MINOR 1
#define VPAINT_OPENGL_VERSION "2.1"

using OpenGLFunctions = QOpenGLFunctions_2_1;
using FrameBufferObjectPtr = std::unique_ptr<QOpenGLExtension_ARB_framebuffer_object>;

OpenGLFunctions* getOpenGLFunctions(QOpenGLContext * context) {
    OpenGLFunctions* gl = context()->versionFunctions<OpenGLFunctions>();
    if (!gl) {
        qFatal("Failed to access OpenGL " VPAINT_OPENGL_VERSION " functions.");
    }
    return gl;
}

void initFrameBufferObject(FrameBufferObjectPtr & fbo, QOpenGLContext * context) {

    // Query extensions
    bool queryExtensions = false;
    if (queryExtensions) {
        QList<QByteArray> extensions = context->extensions().values();
        qDebug() << "Supported extensions (" << extensions.count() << ")";
        foreach (const QByteArray &extension, extensions)
            qDebug() << "    " << extension;
    }

    // Access GL_ARB_framebuffer_object extension
    if (!fbo) {
        if (!context->hasExtension(QByteArrayLiteral("GL_ARB_framebuffer_object"))) {
            qFatal("GL_ARB_framebuffer_object is not supported");
        }
        fbo.reset(new QOpenGLExtension_ARB_framebuffer_object());
        fbo->initializeOpenGLFunctions();
    }
}

#else

#include <QOpenGLFunctions_3_0>

#define VPAINT_OPENGL_VERSION_MAJOR 3
#define VPAINT_OPENGL_VERSION_MINOR 0
#define VPAINT_OPENGL_VERSION "3.0"

using OpenGLFunctions = QOpenGLFunctions_3_0;
using FrameBufferObjectPtr = QOpenGLFunctions_3_0*;

OpenGLFunctions* getOpenGLFunctions(QOpenGLContext * context) {
    OpenGLFunctions* gl = QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_3_0>(context);
    if (!gl) {
        qFatal("Failed to access OpenGL " VPAINT_OPENGL_VERSION " functions.");
    }
    return gl;
}

void initFrameBufferObject(FrameBufferObjectPtr & fbo, QOpenGLContext * context) {
    if (!fbo) {
        fbo = getOpenGLFunctions(context);
    }
}

#endif

#endif
