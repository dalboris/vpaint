#ifndef TESTRUNNER_H
#define TESTRUNNER_H

#include <QObject>
#include <QDir>
#include <QString>
#include <QDateTime>
#include <QProcess>

/// \class TestRunner
/// \brief A class to compile and run tests
///
/// Here is what a TestRunner does:
///
///   1. Generates the following files in outDir:
///          tst_MyTest.h
///          tst_MyTest.pro
///
///   2. Invokes qmake on tst_MyTest.pro, which compiles the test
///      into the following binary file:
///          tst_MyTest     (on Unix)
///          tst_MyTest.exe (on Windows)
///
///   3. Executes the binary, which actually runs the test. Its
///      output is stored as a QString that can be retrieve via out()
///
class TestRunner: public QObject
{
    Q_OBJECT

public:
    /// Enumeration of the possible statuses returned by status().
    ///
    enum class Status
    {
        NotCompiledYet, ///< No attempt to even compile the test yet.
        Compiling,      ///< Currently in the process of compiling the test.
        CompileError,   ///< Last attempt to compile failed.

        NotRunYet,      ///< Last attempt to compile passed. No attempt to run last compiled test yet.
        Running,        ///< Currently in the process of running the test.
        RunError,       ///< Last attempt to compile passed. Last attempt to run failed.
        Pass            ///< Last attempt to compile passed. Last attempt to run passed.
    };

    /// Creates a TestRunner for the test file \p fileName located in the
    /// directory \inDir and to be built in the directory \p outDir.
    ///
    TestRunner(const QDir & inDir,
               const QDir & outDir,
               const QString & fileName,
               QObject * parent = nullptr);

    /// Returns the name of the test.
    ///
    QString testName() const;

    /// Returns the relPath of the test.
    ///
    QString testRelPath() const;

    /// Returns the status of the TestRunner.
    ///
    Status status() const;

    /// Returns the last compile output of the test.
    ///
    QString compileOutput() const;

    /// Returns the last run output of running the test.
    ///
    QString runOutput() const;

    /// Returns a relevant string depending on the TestRunner's status:
    ///   - NotCompiledYet: returns an empty string
    ///   - CompileError:   returns compileOutput()
    ///   - NotRunYet:      returns compileOutput()
    ///   - RunError:       returns runOutput()
    ///   - Pass:           returns runOutput()
    ///
    QString output() const;

public slots:
    /// Compiles the test. Compilation occurs on a separate process, check
    /// status(), or listen for compileFinished().
    ///
    void compile();

    /// Runs the test, compiling beforehand if necessary (i.e., if the
    /// source file of the test has been modified since last compilation).
    /// Run occurs as a separate process, check status(), or listen for
    /// runFinished().
    ///
    void run();

signals:
    /// Signal emitted when compilation finished. the argument \p success
    /// tell whether compilation was successful. If false, then status()
    /// will be CompileError. If true, then status() can be any of
    /// NotRunYet, Running, RunError, or Run.
    ///
    void compileFinished(bool success);

    /// Signal emitted when run finished. the argument \p success
    /// tell whether run was successful. If false, then status()
    /// will be CompileError or RunError. If true, then status()
    /// will be Pass.
    ///
    void runFinished(bool success);

    /// Signal emitted whenever the status changed
    ///
    void statusChanged(Status status);

    /// Signal emitted each time there is new content in the output
    ///
    void outputChanged();

private slots:
    void compile_onQmakeFinished_(int exitCode, QProcess::ExitStatus exitStatus);
    void compile_onMakeFinished_(int exitCode, QProcess::ExitStatus exitStatus);
    void run_onCompileFinished_(bool success);
    void run_onTestFinished_(int exitCode, QProcess::ExitStatus exitStatus);

    void onReadyReadStandardOutput_();

private:
    void setStatus_(Status status);
    void failCompilation_(const QString & errorMessage);

private:
    QDir inDir_;
    QDir outDir_;
    QString fileName_;

    Status status_;
    QString compileOutput_;
    QString runOutput_;

    QDateTime lastCompiled_;
    QString testName_;
    QDir compileDir_;
    QString testBinPath_;
    QProcess * process_;
};

#endif // TESTRUNNER_H
