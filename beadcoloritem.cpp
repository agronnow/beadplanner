#include "beadcoloritem.h"
#include "beadcolortable.h"

BeadColorItem::BeadColorItem(const std::string k, const std::size_t i, Qt::CheckState c, BeadColorItem *parent) : id{k, i}, checkable{true}, checked{c}, parentItem{parent} {}
BeadColorItem::BeadColorItem(const std::string k, const std::size_t i, BeadColorItem *parent) : id{k, i}, checkable{false}, checked{Qt::Unchecked}, parentItem{parent} {}
BeadColorItem::BeadColorItem(const std::string k, const std::size_t i, double d, BeadColorItem *parent) : id{k, i}, colordist{d}, checkable{false}, checked{Qt::Unchecked}, parentItem{parent} {}

BeadColorItem::~BeadColorItem()
{
    qDeleteAll(childItems);
}

void BeadColorItem::appendChild(BeadColorItem *item)
{
    childItems.push_back(item);
}

BeadColorItem* BeadColorItem::child(std::size_t row) const
{
    return childItems[row];
}

std::size_t BeadColorItem::childCount() const
{
    return childItems.size();
}

const std::vector<BeadColorItem*>& BeadColorItem::getChildren() const
{
    return childItems;
}

BeadColorItem* BeadColorItem::getParentItem() const
{
    return parentItem;
}

BeadColorItem* BeadColorItem::findChild(const std::string& key) const
{
    if (childItems.size() == 0) return nullptr;
    auto match = std::find_if(childItems.begin(), childItems.end(), [key](BeadColorItem* item){return item->getKey() == key;});
    if (match != childItems.end())
    {
        return *match;
    }
    else return nullptr;
}

//Returns true if child item was reassigned to other parent
bool BeadColorItem::changeChildParent(BeadColorItem* child, BeadColorItem* newParent)
{
    if (childItems.size() == 0) return false;
    auto match = std::find(childItems.begin(), childItems.end(), child);
    if (match != childItems.end())
    {
        childItems.erase(match);
        //Update indices of subsequent child items (assuming that the BeadColorTable vector element has also been removed)
        for (auto it = match; it != childItems.end(); ++it)
        {
            (*it)->decrementIdx();
        }
        child->setParentItem(newParent);
        newParent->appendChild(child);
        return true;
    }
    else return false;
}

//Returns true if child item was removed, false otherwise
//Note that this will both remove the child from the vector and free its memory
bool BeadColorItem::removeChild(BeadColorItem* child)
{
    auto match = std::find(childItems.begin(), childItems.end(), child);
    if (match != childItems.end())
    {
        childItems.erase(match);
        delete child;
        if (parentItem != nullptr)
        {
            //Update indices of subsequent child items (assuming that the BeadColorTable vector element has also been removed)
            for (auto it = match; it != childItems.end(); ++it)
            {
                (*it)->decrementIdx();
                //std::cout << "decrement " << (*it)->getID().brand << ": " << (*it)->getID().idx << " " << (*it)->getIdx() << std::endl;
            }
        }
        return true;
    }
    else return false;
}

int BeadColorItem::row() const
{
    if (parentItem)
    {
        auto match = std::find(parentItem->childItems.begin(), parentItem->childItems.end(), const_cast<BeadColorItem*>(this));
        return int(std::distance(parentItem->childItems.begin(), match));
    }

    return 0;
}

//Compare two items according to column and order given. Used in sorting.
bool BeadColorItem::compareItemsCount(BeadColorItem* a, BeadColorItem* b, const int column, const BeadColorTable& map, Qt::SortOrder order) const
{
    switch(column)
    {
        case 0:
        {
            if (order == Qt::DescendingOrder) return (map[a].getCode() > map[b].getCode());
            else return (map[a].getCode() < map[b].getCode());
        }
        case 1:
        {
            if (order == Qt::DescendingOrder) return (map[a].getName() > map[b].getName());
            else return (map[a].getName() < map[b].getName());
        }
        case 2:
        {
            if (order == Qt::DescendingOrder) return (a->getKey() > b->getKey());
            else return (a->getKey() < b->getKey());
        }
        case 3:
        {
        if (order == Qt::DescendingOrder) return (map[a].getCount() > map[b].getCount());
        else return (map[a].getCount() < map[b].getCount());
        }
    }
    return false;
}

//Compare two items according to column and order given. Used in sorting.
bool BeadColorItem::compareItemsDist(BeadColorItem* a, BeadColorItem* b, const int column, const BeadColorTable& map, Qt::SortOrder order) const
{
    switch(column)
    {
        case 0:
        {
            if (order == Qt::DescendingOrder) return (map[a].getCode() > map[b].getCode());
            else return (map[a].getCode() < map[b].getCode());
        }
        case 1:
        {
            if (order == Qt::DescendingOrder) return (map[a].getName() > map[b].getName());
            else return (map[a].getName() < map[b].getName());
        }
        case 2:
        {
            if (order == Qt::DescendingOrder) return (a->getKey() > b->getKey());
            else return (a->getKey() < b->getKey());
        }
        case 3:
        {
            if (order == Qt::DescendingOrder) return (a->getDist() > b->getDist());
            else return (a->getDist() < b->getDist());
        }
    }
    return false;
}

//Sorts list of child items according to column and order given and sets permutation vector for updating persistent item indexes.
void BeadColorItem::sortChildren(const int column, std::vector<std::size_t>& index, const BeadColorTable& map, Qt::SortOrder order)
{
    std::iota(index.begin(), index.end(), 0);
    std::function<bool(const int& a, const int& b)> comparison;
    if (getDist() == -1.0) comparison = [&](const std::size_t& a, const std::size_t& b) {return compareItemsCount(childItems[a], childItems[b], column, map, order);};
    else comparison = [&](const std::size_t& a, const std::size_t& b) {return compareItemsDist(childItems[a], childItems[b], column, map, order);};
    std::stable_sort(index.begin(), index.end(), comparison);
    auto newItems(childItems);
    for (std::size_t i = 0; i < index.size(); ++i)
    {
        newItems[i] = childItems[index[i]];
    }
    childItems = newItems;
}

Qt::CheckState BeadColorItem::checkState() const {return checked;} //Check state can be partially checked
void BeadColorItem::setChecked(Qt::CheckState check) {checked = check;} //Check state can be partially checked
void BeadColorItem::setChecked(bool check) {check ? checked = Qt::Checked : checked = Qt::Unchecked;}
bool BeadColorItem::isChecked() const {return checked != Qt::Unchecked;}
bool BeadColorItem::isCheckable(int column) const
{
    return (checkable && (column == 0));
}

