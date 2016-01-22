// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "Application.h"
#include "MainWindow.h"
#include "Global.h"
#include "UpdateCheckDialog.h"

int main(int argc, char *argv[])
{
    Application app(argc, argv);
    MainWindow mainWindow;

    UpdateCheckDialog update(global()->settings().checkVersion());
    update.setParent(&mainWindow, Qt::Dialog);
    if(global()->settings().checkVersion() != Version()) {
        update.checkForUpdates();
    }

    // About window
    if(global()->settings().showAboutDialogAtStartup())
    {
        mainWindow.about();
    }

    update.showWhenReady();

    // Main window
    QObject::connect(&app, SIGNAL(openFileRequested(QString)), &mainWindow, SLOT(doOpen(QString)));
    app.emitOpenFileRequest();
    mainWindow.show();

    return app.exec();
}
