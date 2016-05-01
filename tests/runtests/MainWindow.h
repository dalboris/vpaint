#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class TestItem;
class TestTreeModel;
class TestTreeSelectionModel;
class TestTreeView;
class OutputWidget;

class MainWindow: public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();

private slots:
    void onCurrentTestItemChanged_(TestItem * current, TestItem * previous);

private:
    void updateOutput_();

private:
    TestTreeModel * testTreeModel_;
    TestTreeSelectionModel * testTreeSelectionModel_;
    TestTreeView * testTreeView_;

    OutputWidget * relevantOutputWidget_;
    OutputWidget * compileOutputWidget_;
    OutputWidget * runOutputWidget_;
};

#endif // MAINWINDOW_H
