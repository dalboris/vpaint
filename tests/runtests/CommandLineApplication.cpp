#include "CommandLineApplication.h"
#include "DirUtils.h"
#include "FileTestItem.h"
#include "DirTestItem.h"

#include <QTextStream>
#include <QTimer>

#include <QtDebug>

CommandLineApplication::CommandLineApplication(int & argc, char ** argv) :

    QCoreApplication(argc, argv),

    testItem_(nullptr)
{
    QTextStream out(stdout);

    if (argc < 2)
    {
        out << "Not enough arguments.\n";
    }
    else if (argc > 2)
    {
        out << "Too many arguments.\n";
    }

    if (argc != 2)
    {
        out << "Usage:\n"
            << "    runtests [test-path | all]\n"
            << "\n"
            << "Example 1: (run Gui appplication):\n"
            << "    runtests\n"
            << "/n"
            << "Example 2: (run all tests)\n"
            << "    runtests all\n"
            << "\n"
            << "Example 3: (run all unit tests)\n"
            << "    runtests unit\n"
            << "\n"
            << "Example 4: (run all unit tests in Core)\n"
            << "    runtests unit/Core\n"
            << "\n"
            << "Example 4: (run a given test):\n"
            << "    runtests unit/Core/tst_Foo.cpp\n"
            << "\n";
    }
    else
    {
        QString testPath(argv[1]);

        if (testPath == "all")
        {
            out << "Running all tests...\n";
            testPath = "unit"; // for now, there are only unit tests.
        }
        else if (testPath.endsWith('/'))
        {
            testPath = testPath.mid(0, testPath.size()-1);
        }

        QDir testsDir = DirUtils::dir("tests");

        if (DirUtils::isDir(testsDir, testPath))
        {
            QDir dir    = DirUtils::dir   ("tests/" + testPath);
            QDir outDir = DirUtils::outDir("tests/" + testPath);

            testItem_ = new DirTestItem(dir, outDir, this);

            connect(testItem_, &TestItem::readyReadCommandLineOutput,
                    this, &CommandLineApplication::onReadyReadCommandLineOutput_);

            out << "Running all tests in " << dir.absolutePath() + "...\n";
        }
        else
        {
            QStringList components = testPath.split('/');
            QStringList dirComponents = components.mid(0, components.size()-1);
            QString testDirPath = dirComponents.join('/');
            QString testFileName = components.last() + ".cpp";

            if (DirUtils::isDir(testsDir, testDirPath))
            {
                QDir dir    = DirUtils::dir   ("tests/" + testDirPath);
                QDir outDir = DirUtils::outDir("tests/" + testDirPath);

                if (dir.exists(testFileName))
                {
                    testItem_ = new FileTestItem(dir, outDir, testFileName, this);

                    connect(testItem_, &TestItem::readyReadCompileOutput,
                            this, &CommandLineApplication::onReadyReadCompileOutput_);

                    connect(testItem_, &TestItem::readyReadRunOutput,
                            this, &CommandLineApplication::onReadyReadRunOutput_);

                    out << "Running test " << dir.absoluteFilePath(testFileName) + "...\n";
                }
                else
                {
                    out << "Error: couldn't find " << testFileName
                        << " in " << dir.absolutePath() + "/\n";
                }
            }
            else
            {
                out << "Error: couldn't find the directory " << testsDir.absoluteFilePath(testPath)
                    << "/ or the file " << testsDir.absoluteFilePath(testPath) + ".cpp\n";
            }
        }
    }

    // Run tests once the event loop start (i.e., when app->exec() is called)
    QTimer::singleShot(0, this, SLOT(run_()));
}

void CommandLineApplication::run_()
{
    if (testItem_)
    {
        connect(testItem_, &TestItem::runFinished,
                this, &CommandLineApplication::onRunFinished_);

        testItem_->run();
    }
    else
    {
        quit();
    }
}

void CommandLineApplication::onReadyReadCommandLineOutput_()
{
    QTextStream out(stdout);
    out << testItem_->readCommandLineOutput();
}

void CommandLineApplication::onReadyReadRunOutput_()
{
    QTextStream out(stdout);
    out << testItem_->readRunOutput();
}

void CommandLineApplication::onReadyReadCompileOutput_()
{
    QTextStream out(stdout);
    out << testItem_->readCompileOutput();
}

void CommandLineApplication::onRunFinished_()
{
    if (testItem_->status() == TestItem::Status::Pass)
    {
        quit();
    }
    else
    {
        exit(1);
    }
}
