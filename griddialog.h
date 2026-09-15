#ifndef GRIDDIALOG_H
#define GRIDDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QTabBar>
#include <QPushButton>
#include "grid.h"

//Editor for a single grid's spacing/offset. Keeps a copy of the Grid it was built from so that fields
//not exposed in the UI (style, visibility) survive round-tripping through GridDialog::updateGrids().
class GridTab : public QWidget
{
    Q_OBJECT

public:
    explicit GridTab(const Grid& grid, QWidget *parent = nullptr);
    Grid getGrid() const;
    QSpinBox *gridIntervalXBox;
    QSpinBox *gridIntervalYBox;
    QSpinBox *gridOffsetXBox;
    QSpinBox *gridOffsetYBox;
private:
    Grid grid;
};

class GridDialog : public QDialog
{
    Q_OBJECT
public:
    GridDialog(QWidget*, std::vector<Grid>&);
    void updateGrids();
private slots:
    void addGrid();
    void removeGrid(int index);
private:
    QTabWidget *tabs;
    QPushButton *buttonAddGrid;
    QDialogButtonBox *buttons;
    std::vector<Grid>& oldGrids;
};

#endif // GRIDDIALOG_H
