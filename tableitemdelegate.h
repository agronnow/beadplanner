#ifndef TABLEITEMDELEGATE_H
#define TABLEITEMDELEGATE_H

#include <QStyledItemDelegate>
#include "beadcolortable.h"

class TableItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

    public:
        explicit TableItemDelegate(QRgb, QObject *parent = nullptr);
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        void setColor(QRgb color) {curColor = color;}

    private:
        QRgb curColor;
};

#endif // TABLEITEMDELEGATE_H
