#include "beadcoloritem.h"
#include "treemodel.h"

#include <QStringList>

TreeModel::TreeModel(BeadColorTable& table, QObject *parent)
    : BeadModel(table, 7, parent)
{
    rootItem = new BeadColorItem("Brand,Code,Name,Red,Green,Blue,Source", 0, nullptr);
    beadTable.createTree(rootItem);
}

void TreeModel::rebuild()
{
    emit beginResetModel();
    delete rootItem;
    rootItem = new BeadColorItem("Brand,Code,Name,Red,Green,Blue,Source", 0, nullptr);
    beadTable.createTree(rootItem);
    emit endResetModel();
}

void TreeModel::addItem(const BeadColor& bead, const BeadID& id)
{
    auto parent = rootItem->findChild(id.brand);
    auto rootIndex = index(0,0, QModelIndex());
    if (parent != nullptr)
    {
        //std::cout << "parent pointer" << std::endl;
        auto parentIndex = match(rootIndex, Qt::DisplayRole, QString::fromStdString(id.brand), 1,
                                 Qt::MatchFlags(Qt::MatchFixedString | Qt::MatchCaseSensitive | Qt::MatchWrap));
        if (parentIndex.size() == 0) {std::cout << "add: no parent" << std::endl;return;}
        beginInsertRows(parentIndex[0], int(parent->childCount()), int(parent->childCount()));
        parent->appendChild(new BeadColorItem(id.brand, id.idx, bead.isEnabled() ? Qt::Checked : Qt::Unchecked, parent));
        endInsertRows();
    }
    else
    {
        beginInsertRows(QModelIndex(), int(rootItem->childCount()), int(rootItem->childCount()));
        rootItem->appendChild(new BeadColorItem(id.brand, 0, bead.isEnabled() ? Qt::Checked : Qt::Unchecked, rootItem));
        parent = rootItem->child(rootItem->childCount()-1);
        endInsertRows();
        rootIndex = index(0,0, QModelIndex());
        auto parentIndex = match(rootIndex, Qt::DisplayRole, QString::fromStdString(id.brand), 1,
                                 Qt::MatchFlags(Qt::MatchFixedString | Qt::MatchCaseSensitive | Qt::MatchWrap));
        if (parentIndex.size() == 0) {std::cout << "add: no parent" << std::endl;return;}
        beginInsertRows(parentIndex[0], 0, 0);
        parent->appendChild(new BeadColorItem(id.brand, id.idx, bead.isEnabled() ? Qt::Checked : Qt::Unchecked, parent));
        endInsertRows();
    }
    //emit layoutChanged();
}

void TreeModel::changeItemParent(const QModelIndex& oldIndex, BeadColorItem* item, const BeadID& newID)
{
    auto oldParent = item->getParentItem();
    auto oldParentIndex = oldIndex.parent();
    if (oldParent != nullptr)
    {
        auto newParent = rootItem->findChild(newID.brand);
        if (newParent == nullptr)
        {
            //std::cout << "insert new parent " << oldParentIndex.row() << ", " << oldIndex.row() << std::endl;
            beginInsertRows(oldParentIndex.parent(), int(rootItem->childCount()), int(rootItem->childCount()));
            rootItem->appendChild(new BeadColorItem(newID.brand, 0, Qt::Checked, rootItem));
            newParent = rootItem->child(rootItem->childCount()-1);
            endInsertRows();
        }
        auto newParentIndex = match(oldParentIndex, Qt::DisplayRole, QString::fromStdString(newID.brand), 1,
                                    Qt::MatchFlags(Qt::MatchFixedString | Qt::MatchCaseSensitive | Qt::MatchWrap));
        if (newParentIndex.size() == 0) {std::cout << "remove: no parent" << std::endl;return;}
        //td::cout << "new parent " << newParentIndex[0].row() << ", " << newParentIndex.size() << " : " << oldParentIndex.row() << ", " << oldParentIndex.column() << std::endl;
        beginMoveRows(oldParentIndex, oldIndex.row(), oldIndex.row(), newParentIndex[0], int(newParent->childCount()));
        if (!oldParent->changeChildParent(item, newParent))
        {
            std::cout << "Could not reassign child" << std::endl;
            return;
        }
        endMoveRows();
        if (oldParent->childCount() == 0)
        {
            //std::cout << "remove old parent" << std::endl;
            beginRemoveRows(oldParentIndex.parent(), oldParentIndex.row(), oldParentIndex.row());
            rootItem->removeChild(oldParent);
            endRemoveRows();
        }
        item->setID(newID);
        //std::cout << "key idx " << newKey << ", " << newIdx << " name: " << beadTable[newKey][newIdx].getName() << std::endl;
        //emit layoutChanged();
    }
}

bool TreeModel::removeRows(int row, int count, const QModelIndex& parent)
{
    Q_ASSERT(row >= 0 && count >= 1);
    BeadColorItem *parentItem = static_cast<BeadColorItem*>(parent.internalPointer());
    if ((parentItem != nullptr) && (parentItem->getParentItem() != nullptr))
    {
        beginRemoveRows(parent, row, row+count-1);
        auto item = parentItem->child(std::size_t(row));
        //std::cout << "remove " << item->getID().first << ": " << item->getID().second << std::endl;
        beadTable.remove(item->getID());
        if (!parentItem->removeChild(item)) return false;
        endRemoveRows();
        if (parentItem->childCount() == 0)
        {
            beginRemoveRows(parent.parent(), parent.row(), parent.row());
            if (!parentItem->getParentItem()->removeChild(parentItem)) return false;
            endRemoveRows();

        }
        return true;
    }
    return false;
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    BeadColorItem *item = static_cast<BeadColorItem*>(index.internalPointer());

    if (role == Qt::DisplayRole)
    {
        if (!index.parent().isValid())
        {
            if (index.column() == 0) return QString::fromStdString(item->getKey());
            else return QVariant();
        }
        switch (index.column())
        {
            case 1: return QString::fromStdString(beadTable[item].getCode());
            case 2: return QString::fromStdString(beadTable[item].getName());
            case 3: return QString::number(qRed(beadTable[item].getRGB()));
            case 4: return QString::number(qGreen(beadTable[item].getRGB()));
            case 5: return QString::number(qBlue(beadTable[item].getRGB()));
            case 6: return QString::fromStdString(beadTable[item].getSource());
        }
    }
    else if (role == Qt::DecorationRole)
    {
        if ((index.parent().isValid()) && (index.column() == 0)) return QColor(beadTable[item].getRGB());
    }
    else if (role == Qt::CheckStateRole)
    {
        if (item->isCheckable(index.column()))
        {
            return item->checkState();
        }
        return QVariant();
    }
    return QVariant();
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    if (role == Qt::CheckStateRole)
    {
        BeadColorItem *item = static_cast<BeadColorItem*>(index.internalPointer());
        if (!item->isCheckable(index.column())) return false;

        item->setChecked(value.toInt() == Qt::Checked);
        if (item->childCount() > 0)
        {
            for (const auto& brand : item->getChildren())
            {
                brand->setChecked(value.toInt() == Qt::Checked);
            }
        }
        else if (item->getParentItem() != nullptr)
        {
            //Set check tristate of parent node according to the check states of its children
            std::size_t nChecked = 0;
            for (const auto& brand : item->getParentItem()->getChildren())
            {
                if (brand->isChecked()) nChecked++;
            }
            if (nChecked == item->getParentItem()->childCount()) item->getParentItem()->setChecked(true);
            else if (nChecked == 0) item->getParentItem()->setChecked(false);
            else item->getParentItem()->setChecked(Qt::PartiallyChecked);
        }
        emit layoutChanged();
        return true;
    }
    return false;
}

void TreeModel::onAccept()
{
    //Apply changes to bead color enable/disable states
    for (const auto& brand : rootItem->getChildren())
    {
        auto checkState = brand->checkState();
        if (checkState != Qt::PartiallyChecked)
        {
            for (auto& bead: beadTable[brand->getKey()])
            {
                bead.setEnabled(checkState == Qt::Checked);
            }
        }
        else
        {
            for (const auto& color : brand->getChildren())
            {
                beadTable[color->getID()].setEnabled(color->isChecked());
            }
        }
    }
}
