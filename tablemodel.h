#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include "beadmodel.h"

class TableModel : public BeadModel
{
    Q_OBJECT
public:
    explicit TableModel(BeadColorTable&, int, QObject *parent = nullptr);
    explicit TableModel(BeadColorTable&, QRgb, int, QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role) const override;
    void sort(int column, Qt::SortOrder order) override;
private:
    int pixelsPerBead = 1;
};

#endif // TABLEMODEL_H
