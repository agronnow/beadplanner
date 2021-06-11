#include "tableitemdelegate.h"

TableItemDelegate::TableItemDelegate(QRgb col, QObject *parent) :
    QStyledItemDelegate(parent), curColor{col}
{
}


void TableItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    QVariant data = index.data(Qt::UserRole+1);
    QRgb rgb = data.toUInt();
    opt.font.setBold(rgb == curColor);

    QStyledItemDelegate::paint(painter, opt, index);
}
