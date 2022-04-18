#ifndef TREEMODEL_H
#define TREEMODEL_H

#include "beadmodel.h"

class TreeModel : public BeadModel
{
    Q_OBJECT
public:
    explicit TreeModel(BeadColorTable&, QObject *parent = nullptr);

    void addItem(const BeadColor&, const BeadID&);
    void changeItemParent(const QModelIndex&, BeadColorItem*, const BeadID&);
    void rebuild();
    QVariant data(const QModelIndex&, int) const override;
    bool removeRows(int, int, const QModelIndex &parent = QModelIndex()) override;
    bool setData(const QModelIndex&, const QVariant&, int) override;

public slots:
    void onAccept();
};

#endif // TREEMODEL_H
