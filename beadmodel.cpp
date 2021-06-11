#include "beadmodel.h"
#include "beadcoloritem.h"
#include <QStringList>
#include <iostream>

BeadModel::BeadModel(BeadColorTable& table, int nc, QObject *parent) : QAbstractItemModel(parent), beadTable{table}, nColumns{nc} {}

BeadModel::~BeadModel() {delete rootItem;}

int BeadModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return nColumns;
}

Qt::ItemFlags BeadModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;

    Qt::ItemFlags flags = QAbstractItemModel::flags(index);

    BeadColorItem *item = static_cast<BeadColorItem*>(index.internalPointer());
    if (item->isCheckable(index.column()))
    {
        flags |= Qt::ItemIsUserCheckable;
    }
    if (item->childCount() > 0) flags |= Qt::ItemIsTristate;
    return flags;
}


QVariant BeadModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole))
    {
        QStringList list = QString::fromStdString(rootItem->getKey()).split(',');
        if (section < list.size()) return list[section];
    }

    return QVariant();
}


QModelIndex BeadModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) return QModelIndex();

    BeadColorItem *parentItem;

    if (!parent.isValid()) parentItem = rootItem;
    else parentItem = static_cast<BeadColorItem*>(parent.internalPointer());

    BeadColorItem *childItem = parentItem->child(row);
    if (childItem) return createIndex(row, column, childItem);
    else return QModelIndex();
}

QModelIndex BeadModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) return QModelIndex();

    BeadColorItem *childItem = static_cast<BeadColorItem*>(index.internalPointer());
    BeadColorItem *parentItem = childItem->getParentItem();

    if (parentItem == rootItem) return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int BeadModel::rowCount(const QModelIndex &parent) const
{
    BeadColorItem *parentItem;
    if (parent.column() > 0) return 0;

    if (!parent.isValid()) parentItem = rootItem;
    else parentItem = static_cast<BeadColorItem*>(parent.internalPointer());

    return int(parentItem->childCount());
}
