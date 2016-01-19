// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2015 Connor Deptuck (@scribblemaniac)
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include "Application.h"
#include "MainWindow.h"
#include "Global.h"

int main(int argc, char *argv[])
{
    Application app(argc, argv);
    MainWindow mainWindow;

    // About window
    if(global()->settings().showAboutDialogAtStartup())
    {
        mainWindow.about();
    }

    // Main window
    QObject::connect(&app, SIGNAL(openFileRequested(QString)), &mainWindow, SLOT(doOpen(QString)));
    app.emitOpenFileRequest();
    mainWindow.show();

    return app.exec();
}
