// This file is part of VPaint, a vector graphics editor.
//
// Copyright (C) 2015 Connor Deptuck (@scribblemaniac)
//
// The content of this file is MIT licensed. See COPYING.MIT, or this link:
//   http://opensource.org/licenses/MIT

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>

class Application : public QApplication
{
    Q_OBJECT

public:
    Application(int &argc, char **argv);

    bool event(QEvent* event);
    void emitOpenFileRequest();
signals:
    void openFileRequested(const QString & filename);

private:
    QString startPath_;
};

#endif // APPLICATION_H
