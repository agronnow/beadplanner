#ifndef BEADCOLORITEM_H
#define BEADCOLORITEM_H

#include <QList>
#include <QVariant>
#include <vector>
#include <algorithm>
#include "beadcolor.h"

class BeadColorTable;

//Item representing a bead color, can be used in either a tableview or treeview
//Supports heirarchical and checkable items and sorting
class BeadColorItem
{
public:
    explicit BeadColorItem(const std::string, const std::size_t, Qt::CheckState, BeadColorItem *parentItem = nullptr);
    explicit BeadColorItem(const std::string, const std::size_t, BeadColorItem *parentItem = nullptr);
    explicit BeadColorItem(const std::string, const std::size_t, double, BeadColorItem *parentItem = nullptr);
    ~BeadColorItem();

    void appendChild(BeadColorItem*);

    BeadColorItem* child(std::size_t row) const;
    std::size_t childCount() const;
    void decrementIdx() {if (id.idx>0) id.idx--;}
    const std::vector<BeadColorItem*>& getChildren() const;
    const std::string& getKey() const {return id.brand;}
    std::size_t getIdx() const {return id.idx;}
    void setID(BeadID newID) {id = newID;}
    const BeadID& getID() const {return id;}
    double getDist() const {return colordist;}
    BeadColorItem* findChild(const std::string&) const;
    int row() const;
    bool changeChildParent(BeadColorItem*, BeadColorItem*);
    bool removeChild(BeadColorItem*);
    BeadColorItem* getParentItem() const;
    void setParentItem(BeadColorItem* newParent) {parentItem = newParent;}
    bool compareItemsCount(BeadColorItem*, BeadColorItem*, int, const BeadColorTable&, Qt::SortOrder) const;
    bool compareItemsDist(BeadColorItem*, BeadColorItem*, int, const BeadColorTable&, Qt::SortOrder) const;
    void sortChildren(const int column, std::vector<std::size_t>&, const BeadColorTable&, Qt::SortOrder);
    Qt::CheckState checkState() const;
    void setChecked(Qt::CheckState);
    void setChecked(bool);
    bool isChecked() const;
    bool isCheckable(int) const;

private:
    std::vector<BeadColorItem*> childItems;
    BeadID id;
    double colordist = -1.0;
    bool checkable;
    Qt::CheckState checked;
    BeadColorItem *parentItem;
};

#endif // BEADCOLORITEM_H
