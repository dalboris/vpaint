#ifndef TESTSTREEMODEL_H
#define TESTSTREEMODEL_H

#include <QAbstractItemModel>
#include <QDir>

class TestItem;

class TestTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    TestTreeModel(const QDir & dir,
                  const QDir & outDir,
                  QObject * parent = nullptr);

    QModelIndex indexFromItem(TestItem * item, int column = 0) const;
    TestItem * itemFromIndex(const QModelIndex & index) const;

    QVariant data(const QModelIndex & index, int role) const Q_DECL_OVERRIDE;
    Qt::ItemFlags flags(const QModelIndex & index) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QModelIndex index(int row, int column,
                      const QModelIndex & parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QModelIndex parent(const QModelIndex & index) const Q_DECL_OVERRIDE;
    int rowCount(const QModelIndex & parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex & parent = QModelIndex()) const Q_DECL_OVERRIDE;

private slots:
    void onStatusChanged_(TestItem * item);

private:
    void connectItemSignalsToModelSignals_(const TestItem * item);

private:
    TestItem * rootItem_;

    QDir dir_;
    QDir outDir_;
};
#endif // TESTSTREEMODEL_H
