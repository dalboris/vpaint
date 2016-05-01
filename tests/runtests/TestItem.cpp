#include "TestItem.h"

#include "TestRunner.h"

TestItem::TestItem(QObject * parent) :
    QObject(parent),

    parentItem_(nullptr),
    childItems_(),
    row_(0),

    status_(Status::None),
    progress_(0.0),

    compileOutputReadPos_(0),
    runOutputReadPos_(0),
    commandLineOutputBuffer_()
{
}

TestItem::~TestItem()
{
}

TestItem * TestItem::parentItem() const
{
    return parentItem_;
}

TestItem * TestItem::childItem(int row) const
{
    return childItems_.value(row);
}

int TestItem::numChildItems() const
{
    return childItems_.count();
}

int TestItem::row() const
{
    return row_;
}

void TestItem::appendChildItem(TestItem * childItem)
{
    childItem->row_ = numChildItems();
    childItems_.append(childItem);
    childItem->setParent(this);
    childItem->parentItem_ = this;
}

TestItem::Status TestItem::status() const
{
    return status_;
}

double TestItem::progress() const
{
    return progress_;
}

void TestItem::setStatus(TestItem::Status status)
{
    status_ = status;
    emit statusChanged(this);
}

void TestItem::setProgress(double progress)
{
    progress_ = progress;
    emit progressChanged(this);
}

void TestItem::clearCompileOutput()
{
    compileOutputReadPos_ = 0;
}

void TestItem::clearRunOutput()
{
    runOutputReadPos_ = 0;
}

void TestItem::clearCommandLineOutput()
{
    commandLineOutputBuffer_.clear();
}

void TestItem::appendToCommandLineOutput(const QString & s)
{
    commandLineOutputBuffer_.append(s);
    emit readyReadCommandLineOutput(this);
}

QString TestItem::readCommandLineOutput()
{
    QString res = commandLineOutputBuffer_;
    commandLineOutputBuffer_.clear();
    return res;
}

QString TestItem::readCompileOutput()
{
    QString fullOutput = compileOutput();
    QString unreadOutput = fullOutput.mid(compileOutputReadPos_);
    compileOutputReadPos_ = fullOutput.size();
    return unreadOutput;
}

QString TestItem::readRunOutput()
{
    QString fullOutput = runOutput();
    QString unreadOutput = fullOutput.mid(runOutputReadPos_);
    runOutputReadPos_ = fullOutput.size();
    return unreadOutput;
}
