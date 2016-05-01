#ifndef FILETESTITEM_H
#define FILETESTITEM_H

#include "TestItem.h"
#include "TestRunner.h"

class QDir;

class FileTestItem: public TestItem
{
    Q_OBJECT

public:
    FileTestItem(const QDir & dir,
                 const QDir & outDir,
                 const QString & fileName,
                 QObject * parent = nullptr);

    virtual QString name() const;
    virtual QString statusText() const;

    virtual QString output() const;
    virtual QString compileOutput() const;
    virtual QString runOutput() const;

    virtual void run();

private slots:
    void onOutputChanged_();
    void onStatusChanged_(TestRunner::Status statusText);
    void onRunFinished_(bool success);

private:
    TestRunner * testRunner_;
};

#endif // FILETESTITEM_H
