#include "GuiApplication.h"
#include "MainWindow.h"

GuiApplication::GuiApplication(int & argc, char ** argv) :
    QApplication(argc, argv)
{
    mainWindow_ = new MainWindow();
    mainWindow_->show();
}

GuiApplication::~GuiApplication()
{
    delete mainWindow_;
}
