#ifndef TESTITEM_H
#define TESTITEM_H

#include <QObject>
#include <QList>
#include <QString>

class TestRunner;

/// \class TestItem
/// \brief A class to represent an item in the TestTreeModel
///
/// This is a virtual class. It is inherited by FileTestItem (the leaves of
/// the test tree), and DirTestItem (the non-leaf nodes of the test tree).
///
/// The parentItem of each item (= parent node for test tree) is also its
/// parent QObject (= parent for memory management). The root of the test
/// tree have a null parentItem, but it has a non-null parent QObject.
/// Its parent QObject is the TestTreeModel that manages it.
///
/// Apart from the root item, there is no need to specify the parent
/// QObject of TestItems: both the parentItem and the parent QObject
/// are automatically set when calling appendChildItem().
///
/// When creating a DirTestItem, it auto-populates itself of its child
/// items in the constructor.
///
class TestItem: public QObject
{
    Q_OBJECT

public:
    // Status enumeration
    enum class Status
    {
        None,
        Running,
        Pass,
        Fail,
        FailButStillRunning
    };

    // Constructor and virtual destructor
    TestItem(QObject * parent = nullptr);
    virtual ~TestItem()=0;

    // Parent-child hierarchy
    TestItem * parentItem() const;
    TestItem * childItem(int row) const;
    int numChildItems() const;
    int row() const;

    // Status data.
    // progress() returns how much "run" has progressed between 0.0 and 1.0. Examples:
    //     - 0.0: run hasn't started, or has just started. Status == None or Running.
    //     - 0.5: run in progress, halfway through. Status == Running.
    //     - 1.0: run has finished. Status == Pass or Fail.
    Status status() const;
    double progress() const;

    // Tree view data
    virtual QString name() const=0;
    virtual QString statusText() const=0;

    // Output data
    virtual QString output() const=0;
    virtual QString compileOutput() const=0;
    virtual QString runOutput() const=0;

    // Command line output
    QString readCompileOutput();
    QString readRunOutput();
    QString readCommandLineOutput();

public slots:
    // Run test
    virtual void run()=0;

signals:
    // These signals are automatically emitted by the protected setters
    void statusChanged(TestItem * item);
    void progressChanged(TestItem * item);

    // These signals must be emitted by derived classes when implementing
    // virtual functions.
    void runStarted(TestItem * item);
    void runFinished(TestItem * item);
    void statusTextChanged(TestItem * item);
    void outputChanged(TestItem * item);

    void readyReadCompileOutput(TestItem * item);
    void readyReadRunOutput(TestItem * item);
    void readyReadCommandLineOutput(TestItem * item);

protected:
    // Append a child item to this item.
    void appendChildItem(TestItem * childItem);

    // Changes status to \p status, and emits statusChanged()
    void setStatus(Status status);

    // Changes progress to \p progress, and emits progressChanged()
    void setProgress(double progress);

    // Command line output
    void clearCompileOutput();
    void clearRunOutput();
    void clearCommandLineOutput();
    void appendToCommandLineOutput(const QString & s);

private:
    // Parent-child hierarchy
    TestItem * parentItem_;
    QList<TestItem*> childItems_;
    int row_;

    // Status data
    Status status_;
    double progress_;

    // Command line output
    int compileOutputReadPos_;
    int runOutputReadPos_;
    QString commandLineOutputBuffer_;
};

#endif // TESTSTREEITEM_H
