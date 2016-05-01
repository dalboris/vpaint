#include "FileTestItem.h"

FileTestItem::FileTestItem(
        const QDir & dir,
        const QDir & outDir,
        const QString & fileName,
        QObject *parent) :

    TestItem(parent)
{
    testRunner_ = new TestRunner(dir, outDir, fileName, this);
    connect(testRunner_, &TestRunner::outputChanged, this, &FileTestItem::onOutputChanged_);
    connect(testRunner_, &TestRunner::statusChanged, this, &FileTestItem::onStatusChanged_);
    connect(testRunner_, &TestRunner::runFinished, this, &FileTestItem::onRunFinished_);
}

QString FileTestItem::name() const
{
    return testRunner_->testName();
}

QString FileTestItem::statusText() const
{
    TestRunner::Status status = testRunner_->status();

    switch (status)
    {
    case TestRunner::Status::NotCompiledYet:
        return "";
    case TestRunner::Status::Compiling:
        return "Compiling...";
    case TestRunner::Status::CompileError:
        return "COMPILE ERROR";
    case TestRunner::Status::NotRunYet:
        return "COMPILED";
    case TestRunner::Status::Running:
        return "Running...";
    case TestRunner::Status::RunError:
        return "FAIL";
    case TestRunner::Status::Pass:
        return "PASS";
    }

    return "";
}

QString FileTestItem::output() const
{
    return testRunner_->output();
}

QString FileTestItem::compileOutput() const
{
    return testRunner_->compileOutput();
}

QString FileTestItem::runOutput() const
{
    return testRunner_->runOutput();
}

void FileTestItem::run()
{
    clearCompileOutput();
    clearRunOutput();
    clearCommandLineOutput();
    setProgress(0.0);
    setStatus(Status::Running);
    emit runStarted(this);

    testRunner_->run();
}

void FileTestItem::onOutputChanged_()
{
    emit outputChanged(this);
    emit readyReadCompileOutput(this);
    emit readyReadRunOutput(this);
}

void FileTestItem::onStatusChanged_(TestRunner::Status /*status*/)
{
    emit statusTextChanged(this);
}

void FileTestItem::onRunFinished_(bool success)
{
    setProgress(1.0);
    setStatus(success ? Status::Pass : Status::Fail);

    QString testRelPath = testRunner_->testRelPath();

    switch (testRunner_->status())
    {
    case TestRunner::Status::CompileError:
        appendToCommandLineOutput(compileOutput());
        appendToCommandLineOutput("FAIL: " + testRelPath + "\n");
        break;

    case TestRunner::Status::RunError:
        appendToCommandLineOutput(runOutput());
        appendToCommandLineOutput("FAIL: " + testRelPath + "\n");
        break;

    case TestRunner::Status::Pass:
        appendToCommandLineOutput("PASS: " + testRelPath + "\n");
        break;

    default:
        appendToCommandLineOutput("????: " + testRelPath + "\n");
        break;
    }

    emit runFinished(this);
}
