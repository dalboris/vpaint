#include "TestTreeSelectionModel.h"

#include "TestTreeModel.h"

TestTreeSelectionModel::TestTreeSelectionModel(
        TestTreeModel *model,
        QObject * parent) :

    QItemSelectionModel(model, parent),

    testTreeModel_(model),
    currentTestItem_(nullptr)
{
}

TestItem * TestTreeSelectionModel::currentTestItem() const
{
    return currentTestItem_;
}

void TestTreeSelectionModel::select(const QModelIndex & index,
                                    QItemSelectionModel::SelectionFlags command)
{
    // Correct selection to only allow first column to be selected
    QItemSelectionModel::select(correctedIndex_(index), command);
}

void TestTreeSelectionModel::select(const QItemSelection & selection,
                                    QItemSelectionModel::SelectionFlags command)
{
    // Correct selection to only allow first column to be selected
    QItemSelection correctedSelection;
    foreach(const QItemSelectionRange & range, selection)
    {
        QModelIndex correctedtopLeft = correctedIndex_(range.topLeft());
        QModelIndex correctedbottomRight = correctedIndex_(range.bottomRight());
        QItemSelectionRange correctedRange(correctedtopLeft, correctedbottomRight);
        correctedSelection.append(correctedRange);
    }
    QItemSelectionModel::select(correctedSelection, command);
}

void TestTreeSelectionModel::setCurrentIndex(
        const QModelIndex & index,
        QItemSelectionModel::SelectionFlags command)
{
    // Correct current index to only allow first column to be current
    QItemSelectionModel::setCurrentIndex(correctedIndex_(index), command);

    // Update current test item
    QModelIndex newCurrentIndex = currentIndex();
    TestItem * newCurrentTestItem = testTreeModel_->itemFromIndex(newCurrentIndex);
    if (newCurrentTestItem != currentTestItem_)
    {
        TestItem * previousTestItem = currentTestItem_;
        currentTestItem_ = newCurrentTestItem;
        emit currentTestItemChanged(currentTestItem_, previousTestItem);
    }
}

QModelIndex TestTreeSelectionModel::correctedIndex_(const QModelIndex & index)
{
    // Change index from (row, column, parent) to (row, 0, parent)
    return index.sibling(index.row(), 0);
}
