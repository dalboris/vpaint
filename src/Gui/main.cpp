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

#include <VAC/MainWindow.h>
#include <VAC/Global.h>
#include <VAC/GLUtils.h>

#include "Application.h"
#include "UpdateCheck.h"

int main(int argc, char *argv[])
{

    // Init OpenGL. Must be called before QApplication creation. See Qt doc:
    //
    // Calling QSurfaceFormat::setDefaultFormat() before constructing the
    // QApplication instance is mandatory on some platforms (for example,
    // macOS) when an OpenGL core profile context is requested. This is to
    // ensure that resource sharing between contexts stays functional as all
    // internal contexts are created using the correct version and profile.
    //
    GLUtils::init();


    Application app(argc, argv);
    MainWindow mainWindow;
    UpdateCheck update(&mainWindow);

    // About window
    if(global()->settings().showAboutDialogAtStartup())
    {
        mainWindow.about();
    }

    update.showWhenReady();

    // Main window
    QObject::connect(&app, SIGNAL(openFileRequested(QString)), &mainWindow, SLOT(open_(QString)));
    app.emitOpenFileRequest();
    mainWindow.show();

    return app.exec();
}
