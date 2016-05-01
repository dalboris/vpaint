#ifndef DIRTESTITEM_H
#define DIRTESTITEM_H

#include "TestItem.h"

#include <QDir>
#include <QSet>
#include <QList>

class DirTestItem: public TestItem
{
    Q_OBJECT

public:
    DirTestItem(const QDir & dir,
                const QDir & outDir,
                QObject * parent = nullptr);

    virtual QString name() const;
    virtual QString statusText() const;

    virtual QString output() const;
    virtual QString compileOutput() const;
    virtual QString runOutput() const;

    virtual void run();

private slots:
    void onChildStatusChanged_(TestItem * item);
    void onChildProgressChanged_(TestItem * item);
    void onChildRunStarted_(TestItem * item);
    void onChildRunFinished_(TestItem * item);
    void onChildReadyReadCommandLineOutput_(TestItem * item);

private:
    void updateStatus_();
    void runNextChildItemInQueue_();

private:
    QDir dir_;

    QList<TestItem*> childItemsRunQueue_;
    TestItem * childItemRanByThis_;

    QSet<TestItem*> runningChildItems_;
    QSet<TestItem*> failedChildItems_;
    QSet<TestItem*> passedChildItems_;
};

#endif // DIRTESTITEM_H
