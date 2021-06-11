//Abstract model based on BeadColorItem
//Subclassed by tablemodel and treemodel
//Implements most basic functionality except the data method which must be provided by subclass

#ifndef BEADMODEL_H
#define BEADMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QColor>
#include "beadcolortable.h"

class BeadColorItem;

class BeadModel : public QAbstractItemModel
{
    public:
        explicit BeadModel(BeadColorTable&, int, QObject *parent = nullptr);
        virtual ~BeadModel() override;

        virtual QVariant data(const QModelIndex &index, int role) const override = 0;
        Qt::ItemFlags flags(const QModelIndex &index) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
        QModelIndex parent(const QModelIndex &index) const override;
        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    protected:
        BeadColorItem *rootItem;
        BeadColorTable& beadTable;
        int nColumns;
};

#endif // BEADMODEL_H
