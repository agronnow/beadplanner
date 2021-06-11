#include "beadcoloritem.h"
#include "tablemodel.h"

TableModel::TableModel(BeadColorTable& table, int ppb, QObject *parent)
    : BeadModel(table, 4, parent), pixelsPerBead{ppb}
{
    rootItem = new BeadColorItem(tr("Code,Name,Brand,Count").toStdString(), 0, -1.0, nullptr);
    beadTable.createTable(rootItem);
}

TableModel::TableModel(BeadColorTable& table, QRgb pixelCol, int ppb, QObject *parent)
    : BeadModel(table, 4, parent), pixelsPerBead{ppb}
{
    rootItem = new BeadColorItem(tr("Code,Name,Brand,Colour difference %").toStdString(), 0, 1.0, nullptr);
    beadTable.createTable(rootItem, pixelCol);
}

QVariant TableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    BeadColorItem *item = static_cast<BeadColorItem*>(index.internalPointer());

    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
            case 0: return QString::fromStdString(beadTable[item].getCode());
            case 1: return QString::fromStdString(beadTable[item].getName());
            case 2: return QString::fromStdString(item->getKey());
            case 3:
            {
                //Content of this column depends on whether this is a bead count table or a bead color difference table
                if (rootItem->getDist() == -1.0) return QString::number(int(beadTable[item].getCount()/(pixelsPerBead*pixelsPerBead)));
                else return QString::number(item->getDist(), 'f', 2);
            }
            default: return QVariant();
        }
    }
    else if (role == Qt::DecorationRole)
    {
        if (index.column() == 0) return QColor(beadTable[item].getRGB());
    }
    else if (role == Qt::UserRole) //Querying item key and index for database lookup
    {
        auto id = item->getID();
        return QVariant::fromValue(id);
    }
    else if (role == Qt::UserRole+1) //Querying item RGB color
    {
        return beadTable[item].getRGB();
    }

    return QVariant();
}

void TableModel::sort(int column, Qt::SortOrder order)
{
    emit layoutAboutToBeChanged();
    std::vector<std::size_t> newIdx(rootItem->childCount()); //Permutation vector
    rootItem->sortChildren(column, newIdx, beadTable, order);
    //Update persistent indices according to permutation vector
    QModelIndexList from;
    QModelIndexList to;
    const int numRows = rowCount();
    const int numColumns = columnCount();
    from.reserve(numRows * numColumns);
    to.reserve(numRows * numColumns);
    for (int i = 0; i < numRows; ++i)
    {
        for (int c = 0; c < numColumns; ++c)
        {
            from.append(createIndex(i, c));
            to.append(createIndex(newIdx[i], c));
        }
    }

    changePersistentIndexList(from, to);
    emit layoutChanged();
}
