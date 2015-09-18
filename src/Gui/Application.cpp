// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2015 Connor Deptuck (@scribblemaniac)
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#include <QFileOpenEvent>

#include "Application.h"

Application::Application(int& argc, char** argv) :
    QApplication(argc, argv)
{
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
