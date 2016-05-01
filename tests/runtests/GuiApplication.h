#ifndef GUIAPPLICATION_H
#define GUIAPPLICATION_H

#include <QApplication>

class MainWindow;

class GuiApplication: public QApplication
{
public:
    GuiApplication(int & argc, char ** argv);
    ~GuiApplication();

private:
    MainWindow * mainWindow_;
};

#endif // GUIAPPLICATION_H
