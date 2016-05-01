#include "TestTreeModel.h"
#include "DirTestItem.h"

#include <QMessageBox>

TestTreeModel::TestTreeModel(
        const QDir & dir,
        const QDir & outDir,
        QObject * parent) :

    QAbstractItemModel(parent),
    dir_(dir),
    outDir_(outDir)
{
    rootItem_ = new DirTestItem(dir_, outDir_, this);
    connectItemSignalsToModelSignals_(rootItem_);
}

QModelIndex TestTreeModel::indexFromItem(TestItem * item, int column) const
{
    if (item)
    {
        return createIndex(item->row(), column, item);
    }
    else
    {
        return QModelIndex();
    }
}

TestItem * TestTreeModel::itemFromIndex(const QModelIndex & index) const
{
    if (index.isValid())
    {
        return static_cast<TestItem*>(index.internalPointer());
    }
    else
    {
        return nullptr;
    }
}

QModelIndex TestTreeModel::index(int row, int column, const QModelIndex & parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    if (!parent.isValid())
    {
        return  createIndex(row, column, rootItem_);
    }
    else
    {
        TestItem * parentItem = itemFromIndex(parent);
        if (0 <= row && row < parentItem->numChildItems())
        {
            TestItem * childItem = parentItem->childItem(row);
            return createIndex(row, column, childItem);
        }
        else
        {
            return QModelIndex();
        }
    }
}

QModelIndex TestTreeModel::parent(const QModelIndex & index) const
{
    TestItem * item = itemFromIndex(index);
    TestItem * parentItem = item ? item->parentItem() : nullptr;
    return indexFromItem(parentItem, 0);
}

int TestTreeModel::rowCount(const QModelIndex & parent) const
{
    if (parent.column() > 0)
        return 0;

    if (parent.isValid())
    {
        TestItem * parentItem = static_cast<TestItem*>(parent.internalPointer());
        return parentItem->numChildItems();
    }
    else
    {
        return 1; // One child item: the root item
    }
}

int TestTreeModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 3;
}

QVariant TestTreeModel::data(const QModelIndex & index, int role) const
{
    TestItem * item = itemFromIndex(index);
    if (!item)
        return QVariant();

    if (role == Qt::TextAlignmentRole)
    {
        if (index.column() == 0)
            return Qt::AlignLeft;
        else
            return Qt::AlignCenter;
    }

    if (role != Qt::DisplayRole)
        return QVariant();

    if (index.column() == 0)
        return item->name();
    else if (index.column() == 2)
        return item->statusText();
    else
        return QVariant();
}

Qt::ItemFlags TestTreeModel::flags(const QModelIndex & index) const
{
    if (!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index);
}

QVariant TestTreeModel::headerData(int section, Qt::Orientation orientation,
                                   int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        if (section == 0)
            return "Tests";
        else if (section == 2)
            return "Status";
        else
            return QVariant();
    }
    else
    {
        return QVariant();
    }
}

void TestTreeModel::connectItemSignalsToModelSignals_(const TestItem * item)
{
    connect(item, &TestItem::progressChanged, this, &TestTreeModel::onStatusChanged_);
    connect(item, &TestItem::statusChanged, this, &TestTreeModel::onStatusChanged_);
    connect(item, &TestItem::statusTextChanged, this, &TestTreeModel::onStatusChanged_);
    for (int i=0; i<item->numChildItems(); ++i)
    {
        connectItemSignalsToModelSignals_(item->childItem(i));
    }
}

void TestTreeModel::onStatusChanged_(TestItem * item)
{
    QModelIndex itemIndex = createIndex(item->row(), 2, item);
    emit dataChanged(itemIndex, itemIndex, QVector<int>(Qt::DisplayRole));
}
