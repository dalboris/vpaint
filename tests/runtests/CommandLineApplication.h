#ifndef COMMANDLINEAPPLICATION_H
#define COMMANDLINEAPPLICATION_H

#include <QCoreApplication>

class TestItem;

class CommandLineApplication: public QCoreApplication
{
    Q_OBJECT

public:
    CommandLineApplication(int & argc, char ** argv);

private slots:
    void run_();
    void onRunFinished_();
    void onReadyReadCommandLineOutput_();
    void onReadyReadRunOutput_();
    void onReadyReadCompileOutput_();

private:
    TestItem * testItem_;
};

#endif // COMMANDLINEAPPLICATION_H
