#ifndef GRIDDIALOG_H
#define GRIDDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QTabWidget>
#include "grid.h"

class GridTab : public QWidget
{
    Q_OBJECT

public:
    explicit GridTab(Grid& grid, QWidget *parent = nullptr);
    QSpinBox *gridIntervalXBox;
    QSpinBox *gridIntervalYBox;
    QSpinBox *gridOffsetXBox;
    QSpinBox *gridOffsetYBox;
};

class GridDialog : public QDialog
{
    Q_OBJECT
public:
    GridDialog(QWidget*, std::vector<Grid>&);
    void updateGrids();
private:
    QTabWidget *tabs;
    QDialogButtonBox *buttons;
    std::vector<Grid>& oldGrids;
};

#endif // GRIDDIALOG_H
