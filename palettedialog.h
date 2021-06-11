#ifndef PALETTEDIALOG_H
#define PALETTEDIALOG_H

#include <QDialog>
#include <QTreeView>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QPushButton>
#include "beadcolortable.h"
#include "customcolordialog.h"

class TreeModel;

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

class PaletteDialog : public QDialog
{
    Q_OBJECT
public:
    PaletteDialog(BeadColorTable&, BeadColorTable&, const QStringList&, QWidget*);
    void updatePalette(BeadColorTable&);

private:
    TreeModel *defaultModel;
    QTreeView *defaultColorsTree;
    QTabWidget* tabs;
    QDialogButtonBox *buttons;
};

#endif // PALETTEDIALOG_H
