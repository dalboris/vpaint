// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include "Application.h"
#include "MainWindow.h"

//#include "Global.h"
//#include "UpdateCheck.h"

#include <QFileOpenEvent>
#include <QSurfaceFormat>
#include <QTimer>

Application::Application(int& argc, char** argv) :
    QApplication(argc, argv)
{
    // Set organization and application name
    setOrganizationName("VPaint");
    setOrganizationDomain("vpaint.org");
    setApplicationName("VPaint");
    setApplicationDisplayName("VPaint");

    // Set application version
    setApplicationVersion(APP_VERSION);

    // Init resources
    Q_INIT_RESOURCE(VPaint);

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

    // Create and show main window
    mainWindow_ = new MainWindow();
    mainWindow_->show();

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
}

Application::~Application()
{
    delete mainWindow_;
}

bool Application::event(QEvent* event)
{
    if(event->type() == QEvent::FileOpen)
    {
        startPath_ = static_cast<QFileOpenEvent*>(event)->file();
        emit openFileRequested(startPath_);
        return true;
    }

    return QApplication::event(event);
}

void Application::emitOpenFileRequest()
{
    if(startPath_.size() != 0)
    {
        emit openFileRequested(startPath_);
    }
}
