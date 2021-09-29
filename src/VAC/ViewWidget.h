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

// An ad-hoc class to hold a View + ViewSettings, to workaround the limitation:
//   On Mac OS X, when Qt is built with Cocoa support, a QGLWidget can't
//   have any sibling widgets placed ontop of itself. This is due to limitations
//   in the Cocoa API and is not supported by Apple.
// (cf Qt doc, QGLWidget class)
//
// Therefore, on Mac, the ViewSettings do not lie on top of the view, but as a "toolbar"
//
// Update 2018: The comment above was written before switching from the deprecated
// QGLWidget class to the more modern QOpenGLWidget class. Now, we should be able to
// harmonize the macOS and Windows/Linux versions.

#ifndef VIEWMACOSX_H
#define VIEWMACOSX_H

#include <QWidget>
#include "VAC/vpaint_global.h"

class View;
class ViewSettingsWidget;
class Scene;

// A widget holding a View and a ViewSettingsWidget
//
class Q_VPAINT_EXPORT ViewWidget: public QWidget
{
    Q_OBJECT

public:
    ViewWidget(Scene * scene, QWidget *parent);
    View * view() const;
    ViewSettingsWidget * viewSettingsWidget() const;

private:
    View * view_;
    ViewSettingsWidget * viewSettingsWidget_;
};

#endif // VIEWMACOSX_H
