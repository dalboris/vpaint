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
