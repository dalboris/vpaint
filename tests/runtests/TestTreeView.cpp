#include "TestTreeView.h"
#include "TestTreeModel.h"
#include "TestItem.h"
#include "RunButton.h"

#include <QHeaderView>
#include <cassert>

TestTreeView::TestTreeView(QWidget * parent) :
    QTreeView(parent),
    testTreeModel_(nullptr)
{
    // Set selection mode and behavior
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSelectionBehavior(QAbstractItemView::SelectItems);

    // Set style
    //setAlternatingRowColors(true);
}

void TestTreeView::setModel(QAbstractItemModel * model)
{
    // Ensure we have a non-null TestTreeModel
    testTreeModel_ = dynamic_cast<TestTreeModel*>(model);
    assert(testTreeModel_);

    // Set model
    QTreeView::setModel(testTreeModel_);

    // Make run buttons
    makeRunButtons_();

    // Make root item current
    setCurrentIndex(model->index(0, 0));

    // Set header style and column widths
    header()->setDefaultAlignment(Qt::AlignCenter);
    header()->setStretchLastSection(false);
    header()->setSectionResizeMode(0, QHeaderView::Stretch);
    header()->setSectionResizeMode(1, QHeaderView::Fixed);
    header()->setSectionResizeMode(2, QHeaderView::Fixed);
    header()->resizeSection(1, 16);
    header()->resizeSection(2, 150);
}

TestTreeModel * TestTreeView::testTreeModel() const
{
    return testTreeModel_;
}

void TestTreeView::onRunButtonClicked_(TestItem * item)
{
    setCurrentIndex(testTreeModel()->indexFromItem(item));
}

QModelIndex TestTreeView::firstChild_(const QModelIndex & index)
{
    return model()->index(0, 0, index);
}

QModelIndex TestTreeView::nextSibling_(const QModelIndex & index)
{
    return index.sibling(index.row()+1, 0);
}

void TestTreeView::makeRunButtons_()
{
    QModelIndex rootIndex;
    makeRunButtonsOfChildren_(rootIndex);
}

void TestTreeView::makeRunButtonsOfChildren_(const QModelIndex & parentIndex)
{
    for(QModelIndex index = firstChild_(parentIndex);
        index.isValid();
        index = nextSibling_(index))
    {
        // Get item corresponding to index
        TestItem * item =  testTreeModel()->itemFromIndex(index);

        // Create run button for item
        RunButton * runButton = new RunButton(item);

        // Make item current when run button clicked
        connect(runButton, &RunButton::runClicked, this, &TestTreeView::onRunButtonClicked_);

        // Insert in View
        QModelIndex runButtonIndex = index.sibling(index.row(), 1);
        setIndexWidget(runButtonIndex, runButton);

        // Recurse on children
        makeRunButtonsOfChildren_(index);
    }
}
