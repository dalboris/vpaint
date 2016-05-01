#include "TestRunner.h"
#include "DirUtils.h"
#include "DependsUtils.h"
#include "ConfigUtils.h"

#include <QFile>
#include <QDateTime>
#include <QTextStream>
#include <QStringRef>
#include <QVector>
#include <QProcess>

#include <QtDebug>

namespace
{

QString generateH(const QString & testName, const QString & testSource)
{
    // Find BEGIN_TESTS and END_TESTS
    const QString beginTests = "BEGIN_TESTS";
    const QString endTests   = "END_TESTS";
    const int beginTestsIndex = testSource.indexOf(beginTests);
    const int endTestsIndex   = testSource.indexOf(endTests);

    // Report errors
    if (beginTestsIndex == -1)           return "#error BEGIN_TESTS not found";
    if (endTestsIndex   == -1)           return "#error END_TESTS not found";
    if (endTestsIndex < beginTestsIndex) return "#error END_TESTS appears before BEGIN_TESTS";

    // Get everything before BEGIN_TESTS
    const QString testHeader = testSource.left(beginTestsIndex);

    // Get everything between BEGIN_TESTS and END_TESTS
    const QString testFunctions = testSource.mid(
                beginTestsIndex + beginTests.size(),
                endTestsIndex - beginTestsIndex - beginTests.size());

    // Get everything after END_TESTS
    const QString testFooter = testSource.mid(endTestsIndex + endTests.size());

    // Template string
    QString out =
            "%testHeader"
            "class %testName: public QObject\n"
            "{\n"
            "    Q_OBJECT\n"
            "\n"
            "private slots:"
            "%testFunctions"
            "};"
            "%testFooter";

    // Replace placeholders in template
    out.replace("%testHeader",    testHeader)
       .replace("%testName",      testName)
       .replace("%testFunctions", testFunctions)
       .replace("%testFooter",    testFooter);

    // Replace `#include "Test.h"` by `#include <QTest>`
    out.replace(QRegExp("#include\\s+\"Test.h\""), "#include <QTest>");

    // Return generated tst_Foo.gen.h
    return out;
}

QString generateCpp(const QString & testName, const QString & /*testSource*/)
{
    // Template string
    QString out =
            "#include \"%testName.gen.h\"\n"
            "#include <%appType>\n"
            "\n"
            "int main(int argc, char *argv[])\n"
            "{\n"
            "    %appType app(argc, argv);\n"
            "    QCoreApplication::setAttribute(Qt::AA_Use96Dpi, true);\n"
            "    QTEST_SET_MAIN_SOURCE_PATH\n"
            "    %testName test;\n"
            "    return QTest::qExec(&test, argc, argv);\n"
            "}\n";

    // Replace placeholders in template
    out.replace("%appType",   "QApplication")
       .replace("%testName", testName);

    // Return generated tst_Foo.gen.cpp
    return out;
}

QString generatePro(const QString & testName, const QString & testSource)
{
    // -------- Basic project configuration --------

    QString out =
            "TEMPLATE = app\n"
            "CONFIG += c++11\n"
            "QT += widgets testlib\n"
            "\n"
            "HEADERS += %testName.gen.h\n"
            "SOURCES += %testName.gen.cpp\n";

    out.replace("%testName", testName);


    // -------- Add include paths --------

    QString includePathTemplate;
    if (ConfigUtils::isWin32())
    {
        includePathTemplate =
                "\n"
                "INCLUDEPATH += %thirdDir/\n"
                "INCLUDEPATH += %libsDir/\n";
    }
    else // unix
    {
        includePathTemplate =
                "\n"
                "INCLUDEPATH += %thirdDir/\n"
                "INCLUDEPATH += %libsDir/\n"
                "QMAKE_CXXFLAGS += $$QMAKE_CFLAGS_ISYSTEM %thirdDir/\n";
    }

    includePathTemplate
            .replace("%thirdDir", DirUtils::dir("src/third").absolutePath())
            .replace("%libsDir",  DirUtils::dir("src/libs").absolutePath());

    out += includePathTemplate;


    // -------- Add library dependencies --------

    QString libDependencyTemplate;
    if (ConfigUtils::isWin32())
    {
        libDependencyTemplate =
                "\n"
                "LIBS += -L%libOutDir/%releaseOrDebug/ -l%libName\n"
                "PRE_TARGETDEPS += %libOutDir/%releaseOrDebug/%libName.lib\n";

        libDependencyTemplate.replace("%releaseOrDebug", ConfigUtils::releaseOrDebug());
    }
    else // unix
    {
        libDependencyTemplate =
                "\n"
                "LIBS += -L%libOutDir/ -l%libName\n"
                "PRE_TARGETDEPS += %libOutDir/lib%libName.a\n";
    }

    // Analyse dependencies
    DependsUtils::SDepends sdepends = DependsUtils::getSourceSDepends(testSource);

    // Add qt dependencies
    out += "\nQT += " + sdepends.qt.join(" ") + "\n";

    // Add third dependencies
    for (int i = sdepends.lib.size()-1; i >= 0; --i)
    {
        QString libRelPath = "src/libs/" + sdepends.lib[i];
        QString libOutDir = DirUtils::outDir(libRelPath).absolutePath();
        QString libName = libRelPath.split('/').last();

        QString libDependency = libDependencyTemplate;
        libDependency
                .replace("%libOutDir", libOutDir)
                .replace("%libName",   libName);

        out += libDependency;
    }

    // Add libs dependencies
    for (int i = sdepends.third.size()-1; i >= 0; --i)
    {
        QString libRelPath = "src/third/" + sdepends.third[i];
        QString libOutDir = DirUtils::outDir(libRelPath).absolutePath();
        QString libName = libRelPath.split('/').last();

        QString libDependency = libDependencyTemplate;
        libDependency
                .replace("%libOutDir", libOutDir)
                .replace("%libName",   libName);

        out += libDependency;
    }

    // Return generated tst_Foo.gen.pro
    return out;
}

QString getCurrentTime()
{
    return QTime::currentTime().toString("HH:mm:ss");
}

}

TestRunner::TestRunner(const QDir & inDir,
                       const QDir & outDir,
                       const QString & fileName,
                       QObject * parent) :
    QObject(parent),

    inDir_(inDir),
    outDir_(outDir),
    fileName_(fileName),

    status_(Status::NotCompiledYet),
    compileOutput_(),
    runOutput_()
{
    const QString filePath = inDir_.filePath(fileName_);
    const QFileInfo fileInfo(filePath);
    testName_ = fileInfo.baseName();

    process_ = new QProcess(this);
    process_->setReadChannel(QProcess::StandardOutput);
    process_->setReadChannelMode(QProcess::MergedChannels);
    connect(process_, &QProcess::readyReadStandardOutput, this, &TestRunner::onReadyReadStandardOutput_);
}

QString TestRunner::testName() const
{
    return testName_;
}

TestRunner::Status TestRunner::status() const
{
    return status_;
}


QString TestRunner::compileOutput() const
{
    return compileOutput_;
}

QString TestRunner::runOutput() const
{
    return runOutput_;
}

QString TestRunner::output() const
{
    QString res;

    switch (status())
    {
    case Status::NotCompiledYet:
        res = QString();
        break;

    case Status::Compiling:
    case Status::CompileError:
    case Status::NotRunYet:
        res = compileOutput();
        break;

    case Status::Running:
    case Status::RunError:
    case Status::Pass:
        res = runOutput();
        break;
    }

    return res;
}

void TestRunner::failCompilation_(const QString & errorMessage)
{
    QString time = getCurrentTime();
    compileOutput_ += time + ": Compilation failed: " + errorMessage + "\n";

    emit outputChanged();
    setStatus_(Status::CompileError);
    emit compileFinished(false);
}

void TestRunner::compile()
{
    // Check when is the last time that the test source has been modified
    const QString filePath = inDir_.filePath(fileName_);
    const QFileInfo fileInfo(filePath);
    const QDateTime lastModified = fileInfo.lastModified();

    // Compile if necessary
    Status s = status();
    bool notCompiledYet = (s == Status::NotCompiledYet) || (s == Status::CompileError);
    bool modified       = (lastCompiled_ < lastModified);
    bool processing     = (s == Status::Compiling) || (s == Status::Running);
    if (notCompiledYet || (modified && !processing))
    {
         setStatus_(Status::Compiling);
        compileOutput_.clear();
        lastCompiled_ = lastModified;

        // -------- Go to folder where to compile test --------

        QString compileDirPath = outDir_.absoluteFilePath(testName_);
        compileDir_ = outDir_;
        if (!compileDir_.cd(testName_))
        {
            // Create folder since it doesn't exist
            if (!outDir_.mkdir(testName_))
            {
                failCompilation_("Can't create build folder " + compileDirPath);
                return;
            }

            // Go to folder where to compile test
            // This is hard to reach (if we manage to create the folder, surely
            // we can cd to it), but doesn't hurt to check.
            if (!compileDir_.cd(testName_))
            {
                failCompilation_("Can't create build folder " + compileDirPath);
                return;
            }
        }


        // -------- Open all files for reading or writing --------

        const QString hFileName   = testName_ + ".gen.h";
        const QString cppFileName = testName_ + ".gen.cpp";
        const QString proFileName = testName_ + ".gen.pro";

        const QString hFilePath   = compileDir_.filePath(hFileName);
        const QString cppFilePath = compileDir_.filePath(cppFileName);
        const QString proFilePath = compileDir_.filePath(proFileName);

        QFile sourceFile(filePath);
        QFile hFile(hFilePath);
        QFile cppFile(cppFilePath);
        QFile proFile(proFilePath);

        if (!sourceFile.open(QFile::ReadOnly | QFile::Text))
        {
            failCompilation_("Can't open " + filePath);
            return;
        }

        if (!hFile.open(QFile::WriteOnly | QFile::Text))
        {
            failCompilation_("Can't write " + hFilePath);
            return;
        }

        if (!cppFile.open(QFile::WriteOnly | QFile::Text))
        {
            failCompilation_("Can't write " + cppFilePath);
            return;
        }

        if (!proFile.open(QFile::WriteOnly | QFile::Text))
        {
            failCompilation_("Can't write " + proFilePath);
            return;
        }


        // -------- Read source file --------

        QTextStream sourceStream(&sourceFile);
        const QString testSource = sourceStream.readAll();


        // -------- Generate and write test gen files --------

        QTextStream hStream(&hFile);
        QTextStream cppStream(&cppFile);
        QTextStream proStream(&proFile);

        hStream   << generateH(testName_, testSource);
        cppStream << generateCpp(testName_, testSource);
        proStream << generatePro(testName_, testSource);


        // -------- Run qmake --------

        QString program = QMAKE_QMAKE_QMAKE;
        QStringList arguments;
        arguments << "-spec" << QMAKE_QMAKESPEC << proFilePath;

        compileOutput_ +=
                getCurrentTime() +
                ": Starting: \"" +
                program + "\" " +
                arguments.join(' ') + "\n";

        process_->setWorkingDirectory(compileDirPath);
        connect(process_,
                static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                this,
                &TestRunner::compile_onQmakeFinished_);

        emit outputChanged();

        process_->start(program, arguments);

        // -> go read qMakeFinished_(int exitCode) now.
    }
    else
    {
        emit compileFinished(true);
    }
}

void TestRunner::compile_onQmakeFinished_(int exitCode, QProcess::ExitStatus /*exitStatus*/)
{
    disconnect(process_,
            static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this,
            &TestRunner::compile_onQmakeFinished_);

    compileOutput_ += process_->readAll();
    if (exitCode == 0)
    {
        // -------- Output end of qmake --------

        QString time = getCurrentTime();
        compileOutput_ += time + ": The process \"" + process_->program() + "\" exited normally.\n";


        // -------- Run make --------

        QString program = "make"; // XXX TODO check that this works on Windows too
        QStringList arguments;

        compileOutput_ +=
                getCurrentTime() +
                ": Starting: \"" +
                program + "\" " +
                arguments.join(' ') + "\n";

        connect(process_,
                static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                this,
                &TestRunner::compile_onMakeFinished_);

        emit outputChanged();

        process_->start(program, arguments);

        // -> go read makeFinished_(int exitCode) now.
    }
    else
    {
        QString time = getCurrentTime();
        compileOutput_ +=
                time + ": The process \"" + process_->program() +
                QString("\" exited with code %1.\n").arg(exitCode);
        failCompilation_("qmake failed.");
    }
}


void TestRunner::compile_onMakeFinished_(int exitCode, QProcess::ExitStatus /*exitStatus*/)
{
    disconnect(process_,
            static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this,
            &TestRunner::compile_onMakeFinished_);

    compileOutput_ += process_->readAll();
    if (exitCode == 0)
    {
        QString time = getCurrentTime();
        compileOutput_ += time + ": The process \"" + process_->program() + "\" exited normally.\n";

        testBinPath_ = compileDir_.absoluteFilePath(testName_); // XXX TODO change this on Windows

        emit outputChanged();
        setStatus_(Status::NotRunYet);
        emit compileFinished(true);
    }
    else
    {
        QString time = getCurrentTime();
        compileOutput_ +=
                time + ": The process \"" + process_->program() +
                QString("\" exited with code %1.\n").arg(exitCode);

        failCompilation_("make failed.");
    }
}

void TestRunner::run()
{
    runOutput_.clear();
    connect(this, &TestRunner::compileFinished, this, &TestRunner::run_onCompileFinished_);
    compile();
}

void TestRunner::run_onCompileFinished_(bool success)
{
    disconnect(this, &TestRunner::compileFinished, this, &TestRunner::run_onCompileFinished_);

    if (success)
    {
        setStatus_(Status::Running);

        // -------- Run test --------

        QString program = testBinPath_;
        QStringList arguments;

        runOutput_ +=
                getCurrentTime() +
                ": Starting: \"" +
                program + "\" " +
                arguments.join(' ') + "\n";

        connect(process_,
                static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                this,
                &TestRunner::run_onTestFinished_);

        emit outputChanged();

        process_->start(program, arguments);
    }
    else
    {
        emit runFinished(false);
    }
}



void TestRunner::run_onTestFinished_(int exitCode, QProcess::ExitStatus /*exitStatus*/)
{
    disconnect(process_,
            static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this,
            &TestRunner::run_onTestFinished_);

    runOutput_ += process_->readAll();

    if (exitCode == 0)
    {
        QString time = getCurrentTime();
        runOutput_ += time + ": The process \"" + process_->program() + "\" exited normally.\n";

        emit outputChanged();
        setStatus_(Status::Pass);
        emit runFinished(true);
    }
    else
    {
        QString time = getCurrentTime();
        runOutput_ +=
                time + ": The process \"" + process_->program() +
                QString("\" exited with code %1.\n").arg(exitCode);

        emit outputChanged();
        setStatus_(Status::RunError);
        emit runFinished(false);
    }
}

void TestRunner::setStatus_(Status status)
{
    status_ = status;
    emit statusChanged(status);
}

void TestRunner::onReadyReadStandardOutput_()
{
    switch (status())
    {
    case Status::NotCompiledYet:
    case Status::Compiling:
    case Status::CompileError:
        compileOutput_ += process_->readAll();
        break;

    case Status::NotRunYet:
    case Status::Running:
    case Status::RunError:
    case Status::Pass:
        runOutput_ += process_->readAll();
        break;
    }

    emit outputChanged();
}

QString TestRunner::testRelPath() const
{
    QString testAbsoluteName = inDir_.absoluteFilePath(testName());
    QDir testsDir = DirUtils::dir("tests");
    return testsDir.relativeFilePath(testAbsoluteName);
}
