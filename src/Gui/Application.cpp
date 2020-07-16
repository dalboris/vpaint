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

#include "Application.h"

#include <VAC/Global.h>

#include <QFileOpenEvent>


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
