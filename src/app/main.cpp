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
#include "UpdateCheck.h"

#include <QSurfaceFormat>

int main(int argc, char *argv[])
{
    /*********************** OpenGL configuration ****************************/

    // Set OpenGL 3.2, core profile
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(3, 2);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setSamples(16);
    QSurfaceFormat::setDefaultFormat(format);

    // Share OpenGL context between all QOpenGLWidgets  (i.e., not only those
    // that belong to the same window). See paragraph "Context Sharing" in the
    // QOpenGLWidget documentation. This attribute must be set before
    // instanciating QApplication
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);


    /******************** Create and launch application **********************/

    // Create and initialize application
    Application app(argc, argv);

    // Create and show main window
    MainWindow mainWindow;
    mainWindow.show();

    /* FACTORED_OUT
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
    */

    return app.exec();
}
