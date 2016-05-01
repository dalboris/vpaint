#include "CommandLineApplication.h"
#include "GuiApplication.h"

QCoreApplication * createApplication(int &argc, char *argv[])
{
    bool isGuiApplication = (argc == 1);

    if (isGuiApplication)
        return new GuiApplication(argc, argv);
    else
        return new CommandLineApplication(argc, argv);
}

int main(int argc, char * argv[])
{
    QScopedPointer<QCoreApplication> app(createApplication(argc, argv));
    return app->exec();
}
