#ifndef TESTSTREEVIEW_H
#define TESTSTREEVIEW_H

#include <QTreeView>

class TestItem;
class TestTreeModel;

class TestTreeView: public QTreeView
{
    Q_OBJECT

public:
    TestTreeView(QWidget * parent = nullptr);

    // Set model. The model must be a non-null TestTreeModel.
    virtual void setModel(QAbstractItemModel * model);
    TestTreeModel * testTreeModel() const;

private slots:
    void onRunButtonClicked_(TestItem * item);

private:
    QModelIndex firstChild_(const QModelIndex & index);
    QModelIndex nextSibling_(const QModelIndex & index);
    void makeRunButtons_();
    void makeRunButtonsOfChildren_(const QModelIndex & parentIndex);

private:
    TestTreeModel * testTreeModel_;
};

#endif // TESTSTREEVIEW_H
