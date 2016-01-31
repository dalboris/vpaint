// Copyright (C) 2012-2016 The VPaint Developers.
// See the COPYRIGHT file at the top-level directory of this distribution
// and at https://github.com/dalboris/vpaint/blob/master/COPYRIGHT
//
// This file is part of VPaint, a vector graphics editor. It is subject to the
// license terms and conditions in the LICENSE.MIT file found in the top-level
// directory of this distribution and at http://opensource.org/licenses/MIT

#include <QFileOpenEvent>

#include "Application.h"
#include "Global.h"

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
    connect(this, SIGNAL(aboutToQuit()), this, SLOT(onQuit()));
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

void Application::onQuit()
{
    global()->writeSettings();
}
