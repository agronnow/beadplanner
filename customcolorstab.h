#ifndef CUSTOMCOLORSTAB_H
#define CUSTOMCOLORSTAB_H

#include <QWidget>
#include <QPushButton>
#include <QTreeView>
#include <QHeaderView>
#include "beadcolortable.h"
#include "treemodel.h"
#include "customcolordialog.h"

class CustomColorsTab : public QWidget
{
    Q_OBJECT

public:
    explicit CustomColorsTab(BeadColorTable&, const QStringList&, QWidget *parent = nullptr);
    TreeModel* getModel() const {return customModel;}

signals:
    void dataChanged(const QModelIndex&, const QModelIndex&, const QVector<int> &roles = QVector<int>());

private slots:
    void selectionChanged(const QItemSelection&, const QItemSelection&);
    void addPushed();
    void editPushed();
    void removePushed();

private:
    BeadColorTable& beadTable;
    TreeModel *customModel;
    QStringList keyList;
    QTreeView *customColorsTree;
    QPushButton *buttonAdd;
    QPushButton *buttonEdit;
    QPushButton *buttonRemove;
};


#endif // CUSTOMCOLORSTAB_H
