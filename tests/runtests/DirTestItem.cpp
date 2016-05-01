#include "DirTestItem.h"
#include "FileTestItem.h"
#include "DirUtils.h"

DirTestItem::DirTestItem(
        const QDir & dir,
        const QDir & outDir,
        QObject * parent) :

    TestItem(parent),

    dir_(dir),

    childItemsRunQueue_(),
    childItemRanByThis_(nullptr),

    runningChildItems_(),
    failedChildItems_(),
    passedChildItems_()
{
    // Find subdirs
    QFileInfoList subdirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

    // Append subdir child items
    for (int i = 0; i < subdirs.size(); ++i)
    {
        // Get subdir name
        QString name = subdirs[i].fileName();

        // Cd to subdir dirs (this implicitely calls mkdir if necessary)
        QDir subdirDir = dir;
        QDir subdirOutDir = outDir;
        if (!DirUtils::cd(subdirDir, name) || !DirUtils::cd(subdirOutDir, name))
            continue;

        // Create child item
        DirTestItem * item = new DirTestItem(subdirDir, subdirOutDir);

        // Append child
        appendChildItem(item);
    }

    // Find test files
    QStringList filters;
    filters << "tst_*.cpp";
    QFileInfoList testFiles = dir.entryInfoList(filters, QDir::Files, QDir::Name);

    // Append test file child items
    for (int i = 0; i < testFiles.size(); ++i)
    {
        // Get fileName
        QString fileName = testFiles[i].fileName();

        // Create child item
        FileTestItem * item = new FileTestItem(dir, outDir, fileName);

        // Append child
        appendChildItem(item);
    }

    // Connect with children
    for (int i=0; i<numChildItems(); ++i)
    {
        TestItem * child = childItem(i);
        connect(child, &TestItem::statusChanged,   this, &DirTestItem::onChildStatusChanged_);
        connect(child, &TestItem::progressChanged, this, &DirTestItem::onChildProgressChanged_);
        connect(child, &TestItem::runStarted,      this, &DirTestItem::onChildRunStarted_);
        connect(child, &TestItem::runFinished,     this, &DirTestItem::onChildRunFinished_);

        connect(child, &TestItem::readyReadCommandLineOutput,
                this, &DirTestItem::onChildReadyReadCommandLineOutput_);
    }
}

QString DirTestItem::name() const
{
    return dir_.dirName();
}

QString DirTestItem::statusText() const
{
    const int p = 100 * progress();
    const int fieldWidth = 2;
    const int base = 10;
    const QChar fillChar = '0';

    switch (status())
    {
    case Status::None:
        return "";
    case Status::Pass:
        return "PASS";
    case Status::Fail:
        return "FAIL";
    case Status::Running:
        return QString("Running... (%1%)").arg(p, fieldWidth, base, fillChar);
    case Status::FailButStillRunning:
        return QString("FAIL... (%1%)").arg(p, fieldWidth, base, fillChar);
    }

    return "";
}

QString DirTestItem::output() const
{
    return "";
}

QString DirTestItem::compileOutput() const
{
    return "";
}

QString DirTestItem::runOutput() const
{
    return "";
}

void DirTestItem::run()
{
    setProgress(0.0);
    setStatus(Status::Running);
    emit runStarted(this);

    childItemsRunQueue_.clear();
    for (int i=0; i<numChildItems(); ++i)
    {
        childItemsRunQueue_ << childItem(i);

        // An other option tried was:
        //     childItem(i)->run();
        //
        // and keep track, in onChildRunFinished_, of how many child were
        // still runnning. This introduced useful parallelism,
        // but unfortunately didn't scale: I tried with 100 unit tests,
        // which means 100 processes were launched in parallel all at once,
        // and that crashed my Linux system. And 100 is not that many tests,
        // it could be easily reached by any medium-scale project, and
        // obviously reached by large-scale projects.
        //
        // So for the time being, let's run them sequentially. In the future,
        // it would be useful to re-introduce some parallelism but limiting
        // how many processes are launched at once to the number of processors
        // in the host computer (or to a number specified by the user).
    }

    runNextChildItemInQueue_();
}

void DirTestItem::runNextChildItemInQueue_()
{
    updateStatus_();

    if (childItemsRunQueue_.isEmpty()) // Finished!
    {
        QString testAbsolutePath = dir_.absolutePath();
        QDir testsDir = DirUtils::dir("tests");
        QString testRelPath = testsDir.relativeFilePath(testAbsolutePath);

        switch (status())
        {
        case TestItem::Status::Fail:
            appendToCommandLineOutput("FAIL: " + testRelPath + "\n");
            break;

        case TestItem::Status::Pass:
            appendToCommandLineOutput("PASS: " + testRelPath + "\n");
            break;

        default:
            appendToCommandLineOutput("????: " + testRelPath + "\n");
            break;
        }

        emit runFinished(this);
    }
    else // Still some children to run
    {
        TestItem * child = childItemsRunQueue_.front();
        childItemsRunQueue_.pop_front();
        childItemRanByThis_ = child;
        child->run();
    }
}

void DirTestItem::updateStatus_()
{
    if (status() != Status::None)
    {
        bool isPassed =
                (passedChildItems_.size() == numChildItems());

        bool isFailed =
                (failedChildItems_.size() > 0);

        bool isRunning =
                (childItemsRunQueue_.size() > 0) ||
                (runningChildItems_.size()  > 0);

        Status newStatus;
        if (isPassed)
        {
            newStatus = Status::Pass;
        }
        else
        {
            if (isRunning)
            {
                if (isFailed)
                {
                    newStatus = Status::FailButStillRunning;
                }
                else
                {
                    newStatus = Status::Running;
                }
            }
            else
            {
                if (isFailed)
                {
                    newStatus = Status::Fail;
                }
                else
                {
                    newStatus = Status::None;
                }
            }
        }

        setStatus(newStatus);
    }
}

void DirTestItem::onChildStatusChanged_(TestItem * item)
{
    // Insert and/or remove child item from relevant sets
    switch (item->status())
    {
    case Status::None:
        break;
    case Status::Running:
        runningChildItems_.insert(item);
        failedChildItems_.remove(item);
        passedChildItems_.remove(item);
        break;
    case Status::Pass:
        runningChildItems_.remove(item);
        failedChildItems_.remove(item);
        passedChildItems_.insert(item);
        break;
    case Status::Fail:
        runningChildItems_.remove(item);
        failedChildItems_.insert(item);
        passedChildItems_.remove(item);
        break;
    case Status::FailButStillRunning:
        runningChildItems_.insert(item);
        failedChildItems_.insert(item);
        passedChildItems_.remove(item);
        break;
    }

    // Update status
    updateStatus_();
}

void DirTestItem::onChildProgressChanged_(TestItem * /*item*/)
{
    // Compute progress from progresses of child items
    double progress = 0.0;
    for (int i=0; i<numChildItems(); ++i)
        progress += childItem(i)->progress();
    progress /= numChildItems();

    // Set progress
    setProgress(progress);
}

void DirTestItem::onChildRunStarted_(TestItem * /*item*/)
{
    // nothing do do
}

void DirTestItem::onChildRunFinished_(TestItem * item)
{
    if (item == childItemRanByThis_) // prevent messing up child items auto-ran by
                                     // parent with child items manually ran by user
    {
        childItemRanByThis_ = nullptr;
        runNextChildItemInQueue_();
    }
}

void DirTestItem::onChildReadyReadCommandLineOutput_(TestItem * item)
{
    appendToCommandLineOutput(item->readCommandLineOutput());
}
