#ifndef TESTTREESELECTIONMODEL_H
#define TESTTREESELECTIONMODEL_H

#include <QItemSelectionModel>

class TestItem;
class TestTreeModel;

class TestTreeSelectionModel: public QItemSelectionModel
{
    Q_OBJECT

public:
    TestTreeSelectionModel(TestTreeModel * model,
                           QObject * parent = nullptr);

    TestItem * currentTestItem() const;

signals:
    void currentTestItemChanged(TestItem * current, TestItem * previous);

public slots:
    virtual void select(const QModelIndex & index, QItemSelectionModel::SelectionFlags command);
    virtual void select(const QItemSelection & selection, QItemSelectionModel::SelectionFlags command);
    virtual void setCurrentIndex(const QModelIndex & index, QItemSelectionModel::SelectionFlags command);

private:
    QModelIndex correctedIndex_(const QModelIndex & index);

private:
    TestTreeModel * testTreeModel_;
    TestItem * currentTestItem_;
};

#endif // TESTTREESELECTIONMODEL_H
